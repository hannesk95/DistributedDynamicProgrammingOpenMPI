#include "vi_processor_impl_distr_02.h"
#include <limits>
#include <mpi.h>


void VI_Processor_Impl_Distr_02::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    )
{
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    // Split up states for each process (last process gets additional remainder)
    const int process_first_state = (J.size() / world_size) * world_rank;
    const int process_last_state =  (world_size == world_rank + 1) ? J.size() : (J.size() / world_size) * (world_rank + 1);

    // same received J and Pi
    std::vector<float> J_recv_buffer(0);
    std::vector<int> Pi_recv_buffer(0);
    // Pi and J have the exact same size, so buffer_size is the same for both as well!
    const int remainder = (J.size() % world_size);
    const int buffer_size = (J.size() / world_size);
    // root, only it receives data
    if (world_rank == root_id)
    {
        J_recv_buffer.resize(buffer_size + remainder);
        Pi_recv_buffer.resize(buffer_size + remainder);
    }

    // Keeps track of the change in J vector
    float error = 0;

    for(unsigned int t=0; t < max_iter; ++t)
    {
        // Compute one value iteration step for a range of states
        float max_diff = iteration_step(Pi, J, P, process_first_state, process_last_state);

        // Store value of biggest change that appeared while updating J
        if(max_diff > error) error = max_diff;

        // Exchange results every comm_period cycles
        if(t % comm_period == 0)
        {
            // all non-roots are sending to root
            if (world_rank != 0)
            {
                float* J_raw = J.data();
                MPI_Send(J_raw + process_first_state, process_last_state - process_first_state, MPI_FLOAT,  root_id, 0, MPI_COMM_WORLD); // MPI_ANY_TAG is only valid for recv
            }
            // root receives
            else {
                // receive from all except root
                for (int i = 0; i < world_size - 1; ++i) {
                    // find out who is sending! Wait for the first message that occurs
                    MPI_Status status;
                    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    int source_id = status.MPI_SOURCE;
                    // get payload size
                    auto payload_size = buffer_size;
                    // last one has a little bit more payload!
                    if (source_id == world_size - 1) {
                        payload_size += remainder;
                    }
                    // now receive the sub vector into J receive buffer
                    MPI_Recv(J_recv_buffer.data(), payload_size, MPI_FLOAT, source_id, MPI_ANY_TAG, MPI_COMM_WORLD,
                             MPI_STATUS_IGNORE);
                    // Generate Eigen style vector of received data
                    Eigen::Map<Eigen::VectorXf> J_recv(J_recv_buffer.data(), payload_size);
                    // get difference between J_recv and corresponding segment in J
                    float recv_max_diff = (J.segment(source_id * buffer_size, payload_size) -
                                           J_recv).cwiseAbs().maxCoeff();
                    // check if new maximum difference -> if yes overwrite
                    if (recv_max_diff > error) error = recv_max_diff;
                    // update J
                    J.segment(source_id * buffer_size, payload_size) = J_recv;
                } // root has now maximum error value and new J vector
            }
            // Broadcast error from root to all processes (that they can stop respectively), also synchronizes all processes
            MPI_Bcast(&error, 1, MPI_FLOAT, root_id, MPI_COMM_WORLD);
            // If convergence rate is below threshold stop
            if(error <= tolerance)
            {
                debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                break;
            }
            // Broadcast J again from root to all processes because we did not converge yet
            MPI_Bcast(J.data(), J.size(), MPI_FLOAT, root_id, MPI_COMM_WORLD);
            // Reset change rate to 0 such that it can be overwritten with a new (smaller) value
            error = 0;
        }
    }
    // now merge policy accordingly to J

    // all non-roots are sending to root
    if (world_rank != 0)
    {
        int* Pi_raw = Pi.data();
        MPI_Send(Pi_raw + process_first_state, process_last_state - process_first_state, MPI_INT, root_id, 0, MPI_COMM_WORLD); // MPI_ANY_TAG is only valid for recv
    }
    // root receives
    else
    {
        // receive from all except root
        for (int i = 0; i < world_size - 1; ++i)
        {
            // find out who is sending! Wait for the first message that occurs
            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int source_id = status.MPI_SOURCE;
            // get payload size
            auto payload_size = buffer_size;
            // last one has a little bit more payload!
            if (source_id == world_size - 1)
            {
                payload_size += remainder;
            }
            // now receive the sub vector into Pi receive buffer
            MPI_Recv(Pi_recv_buffer.data(), payload_size, MPI_INT, source_id, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Generate Eigen style vector of received data
            Eigen::Map<Eigen::VectorXi> Pi_recv(Pi_recv_buffer.data(), payload_size);
            // update Pi
            Pi.segment(source_id * buffer_size, payload_size) = Pi_recv;
        }
    }
}