#include "vi_processor_impl_distr_03.h"
#include <limits>
#include <mpi.h>


void VI_Processor_Impl_Distr_03::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int T)
{
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    MPI_Status status;
    MPI_Request request;

    // Split up states for each process (last process gets additional remainder)
    const int process_first_state = (J.size() / world_size) * world_rank;
    const int process_last_state =  (world_size == world_rank + 1) ? J.size() : (J.size() / world_size) * (world_rank + 1);
    

    //std::vector<float> J_sub(process_last_state - process_first_state, 0.0);






    for(unsigned int t=0; t < T; ++t)
    {
        std::vector<float> J_raw;
        std::vector<float> J_sub;

        // Compute one value iteration step for a range of states
        iteration_step(Pi, J, P, process_first_state, process_last_state);



        // Exchange results every comm_period cycles
        if(t % comm_period == 0)
        {           
            if ( world_rank != 0)
            {
                float* J_raw = J.data();

                MPI_Isend(
                    &J_raw[process_first_state],
                    process_last_state - process_first_state,
                    MPI_FLOAT,
                    0,
                    0,
                    MPI_COMM_WORLD,
                    &request
                );

                MPI_Wait(&request, &status);
            }          

            else
            {
                //J_raw.insert(J_raw.begin(), J_sub);
                //J_raw.insert(J_raw.begin(), J_sub.begin(), J_sub.end());
                //int end = process_last_state - process_first_state;

                for(int i = 0; i < J_sub.size(); i++)
                {
                    J_raw.push_back(J_sub[i]);
                }

                
                for(int i = 1; i < world_size - 1; i++)
                {
                    
                    MPI_Irecv(
                        &J_sub,
                        process_last_state - process_first_state,
                        MPI_FLOAT,
                        i,
                        0, 
                        MPI_COMM_WORLD,
                        &request
                    );                    

                    // Blocks and waits for destination process to receive data
                    MPI_Wait(&request, &status);

                    //int begin_offset = i*(process_last_state-process_first_state);

                    //J_raw.insert(J_raw.begin() + begin_offset, J_sub.begin(), J_sub.end());

                    for(int j = 0; i < J_sub.size(); i++)
                    {
                        J_raw.push_back(J_sub[j]);
                    }

                }

                if(world_size != 1)
                {
                    MPI_Irecv(
                            &J_sub,
                            J.size() - (process_last_state-process_first_state)*(world_size-1),
                            MPI_FLOAT,
                            world_rank-1,
                            0,
                            MPI_COMM_WORLD,
                            &request
                    );

                    // Blocks and waits for destination process to receive data
                    MPI_Wait(&request, &status);

                    //int begin_offset = (world_size-1)*(process_last_state-process_first_state);
                    //J_raw.insert(J_raw.begin() + begin_offset, J_sub.begin(), J_sub.end());

                    for(int i = 0; i < J_sub.size(); i++)
                    {
                        J_raw.push_back(J_sub[i]);
                    }
                }
                
                // Map std::vector into Eigen::Vector
                Eigen::Map<Eigen::VectorXf> J_new(J_raw.data(), J_raw.size());

                // Calculate error and pick maximum
                auto error = (J_new - J).cwiseAbs().maxCoeff();

                // If convergence criteria is reached -> terminate
                if(error <= e_max)
                {
                    debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                    break;
                } 

                // Else: Broadcast J_raw to all processors as algorithm did not converge
                MPI_Bcast(
                    J_new.data(),
                    J_new.size(),
                    MPI_FLOAT,
                    0,
                    MPI_COMM_WORLD
                );                 
            }           
        }
    }

//    std::vector<int> recvcounts;    // Number of states for each process
//    std::vector<int> displs;        // Displacement of sub vectors for each process
//    for(int i=0; i < world_size; ++i)
//    {
//        if(i+1 < world_size) recvcounts.push_back((J.size() / world_size));
//        else recvcounts.push_back((J.size() / world_size) + J.size() % world_size);
//
//        if(i == 0) displs.push_back(0);
//        else displs.push_back(displs[i-1] + recvcounts[i-1]);
//    }
//
//    // Merge Policy
//    int* Pi_raw = Pi.data();
//    MPI_Gatherv(
//            &Pi_raw[process_first_state],
//            process_last_state - process_first_state,
//            MPI_INT,
//            Pi_raw,
//            recvcounts.data(),
//            displs.data(),
//            MPI_INT,
//            0,
//            MPI_COMM_WORLD
//    );
}