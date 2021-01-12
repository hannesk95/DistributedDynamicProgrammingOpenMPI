#include "vi_processor_impl_distr_02.h"
#include <limits>
#include <mpi.h>


void VI_Processor_Impl_Distr_02::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int T
    )
{
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    // Split up states for each process (last process gets additional remainder)
    const int process_first_state = (J.size() / world_size) * world_rank;
    const int process_last_state =  (world_size == world_rank + 1) ? J.size() : (J.size() / world_size) * (world_rank + 1);

    // define variables that root needs
    const int last_state = world_size - 1;
    const int workload = (J.size() / world_size);
    const int last_workload = J.size() - (workload * last_state);
    const int sub_size = J.size() / world_size;
    Eigen::VectorXf J_new(J.size());
    J_new.fill(0);
    Eigen::VectorXf J_sub(sub_size);

    // Keeps track of the change in J vector
    float error = 0;

    for(unsigned int t=0; t < T; ++t)
    {
        // Compute one value iteration step for a range of states
        float max_diff = iteration_step(Pi, J, P, process_first_state, process_last_state);

        // Store value of biggest change that appeared while updating J
        if(max_diff > error) error = max_diff;

        // Exchange results every comm_period cycles
        if(t % comm_period == 0)
        {
            if (world_rank != root_id)
            {
                float* J_new = J.data();
                MPI_Send(&J_new[process_first_state], process_last_state - process_first_state, MPI_FLOAT, root_id, 0, MPI_COMM_WORLD);
            }
            else
            {
                // Synchronize all workers
                MPI_Barrier(MPI_COMM_WORLD);
                // set first piece
                J_new.segment(0, workload) = J.segment(0, workload);
                for (int i = 1; i < last_state; i++)
                {
                  MPI_Recv(&J_sub, workload, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                  J_new.segment(workload * i, workload) = J_sub;
                }
                MPI_Recv(&J_sub, last_workload, MPI_FLOAT, last_state, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                J_new.segment(workload * last_state, last_workload) = J_sub;
                MPI_Bcast(J_new.data(), J_new.size(), MPI_FLOAT, root_id, MPI_COMM_WORLD);
            
                // Check for convergence
                auto J_abs_diff = (J - J_new).cwiseAbs();
                error = J_abs_diff.maxCoeff();

                // Overwrite J
                J = J_new;

                // If convergence rate is below threshold stop
                if(error <= e_max)
                {
                    debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                    break;
                }

                // Reset change rate to 0 such that it can be overwritten with a new (smaller) value
                error = 0;
            }
        }

    }

    std::vector<int> recvcounts;    // Number of states for each process
    std::vector<int> displs;        // Displacement of sub vectors for each process
    for(int i=0; i < world_size; ++i)
    {
        if(i+1 < world_size) recvcounts.push_back((J.size() / world_size));
        else recvcounts.push_back((J.size() / world_size) + J.size() % world_size);

        if(i == root_id) displs.push_back(0);
        else displs.push_back(displs[i-1] + recvcounts[i-1]);
    }

    // Merge Policy
    int* Pi_raw = Pi.data();
    MPI_Gatherv(&Pi_raw[process_first_state], 
                process_last_state - process_first_state,
                MPI_INT, 
                Pi_raw, 
                recvcounts.data(), 
                displs.data(), 
                MPI_INT, 
                root_id, 
                MPI_COMM_WORLD
    );
}