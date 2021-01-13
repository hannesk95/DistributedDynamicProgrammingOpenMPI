#include "vi_processor_impl_distr_04.h"
#include <limits>
#include <mpi.h>
#include <math.h>


void VI_Processor_Impl_Distr_04::value_iteration_impl(
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

    int processor_workload = ceil(J.size() / world_size);
    int processor_workload_last = processor_workload + (J.size() % world_size);
    int processor_start = processor_workload * world_rank;
    int processor_end = processor_workload * (world_rank +1);

    Eigen::VectorXf J_processor(0);

    if (world_rank == 0)
    {
        Eigen::VectorXf J_merged(J.size());
        J_processor.resize(processor_workload_last);
    }


    float error = 0;


    for(unsigned int t=0; t < T; ++t)
    {
        iteration_step(Pi, J, P, processor_start, processor_end);

        if(t % comm_period == 0)
        {           
            // All slaves (world_rank > 0)
            if ( world_rank != 0)
            {
                float* J_raw = J.data();

                MPI_Isend(J_raw + processor_start,
                    processor_workload,
                    MPI_FLOAT,
                    root_id,
                    0,
                    MPI_COMM_WORLD,
                    &request
                );

                MPI_Wait(&request, &status);
            }          

            // Only master (world_rank == 0)
            else
            {
                for(int i = 0; i < world_size - 1; i++)
                {

                    MPI_Probe(MPI_ANY_SOURCE,
                              MPI_ANY_TAG,
                              MPI_COMM_WORLD,
                              &status);

                    int source_rank = status.MPI_SOURCE;

                    if (source_rank == world_rank - 1)
                    {
                        processor_workload = processor_workload_last;
                    }

                    MPI_Irecv(J_processor.data(),
                        processor_workload,
                        MPI_FLOAT,
                        source_rank,
                        MPI_ANY_TAG,
                        MPI_COMM_WORLD,
                        &request
                    );                    

                    // Blocks and waits for destination process to receive data
                    MPI_Wait(&request, &status);

                    float recv_max_diff = (J.segment(source_rank * processor_workload, processor_workload) -
                                           J_processor).cwiseAbs().maxCoeff();

                    if (recv_max_diff > error)
                    {
                        error = recv_max_diff;
                    }

                    J.segment(source_rank * processor_workload, processor_workload) = J_processor;
                }
                
                MPI_Bcast(&error, 1, MPI_FLOAT, root_id, MPI_COMM_WORLD);

                // If convergence criteria is reached -> terminate
                if(error <= e_max)
                {
                    debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                    break;
                }

                MPI_Bcast(J.data(), J.size(), MPI_FLOAT, root_id, MPI_COMM_WORLD);

                error = 0;
            }           
        }
    }
}