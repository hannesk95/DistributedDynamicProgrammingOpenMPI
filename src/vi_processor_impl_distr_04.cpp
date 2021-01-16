#include "vi_processor_impl_distr_04.h"
#include <limits>
#include <mpi.h>
#include <math.h>

#define MPI_Error_Check(x) {const int err=x; if(x!=MPI_SUCCESS) { fprintf(stderr, "MPI ERROR %d at %d.", err, __LINE__);}}


bool VI_Processor_Impl_Distr_04::SetParameter(std::string param, float value)
{
    if(param == "comm_period")
    {
        comm_period = value;
        return true;
    }

    return VI_Processor_Base::SetParameter(param, value);
}

std::map<std::string, float> VI_Processor_Impl_Distr_04::GetParameters()
{
    std::map<std::string, float> parameters = VI_Processor_Base::GetParameters();
    parameters["comm_period"] = comm_period;
    return parameters;
}


void VI_Processor_Impl_Distr_04::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int T)
{
    int world_size, world_rank;
    MPI_Error_Check(MPI_Comm_size(MPI_COMM_WORLD, &world_size));
    MPI_Error_Check(MPI_Comm_rank(MPI_COMM_WORLD, &world_rank));

    int request_complete = 0;   // Use if MPI_Test is used

    int processor_workload = ceil(J.size() / world_size);
    int processor_workload_last = processor_workload + (J.size() % world_size);
    int processor_start = processor_workload * world_rank;
    int processor_end = processor_workload * (world_rank +1);

    // Eigen::VectorXf J_processor(0);
    // Eigen::VectorXf Pi_processor(0);
    std::vector<float> J_processor(0);
    std::vector<float> Pi_processor(0);

    if (world_rank == 0)
    {
        // Eigen::VectorXf J_merged(J.size());
        J_processor.resize(processor_workload_last);
        Pi_processor.resize(processor_workload_last);
    }

    float error = 0;

    for(unsigned int t=0; t < T; ++t)
    {
        float max_diff = iteration_step(Pi, J, P, processor_start, processor_end);

        if(max_diff > error)
        {
            error = max_diff;
        }

        if(t % comm_period == 0)
        {
            MPI_Status status;
            MPI_Request request;

            if ( world_rank != root_id)
            {
                float* J_raw = J.data();

                MPI_Error_Check(MPI_Isend(J_raw + processor_start,
                    processor_workload,
                    MPI_FLOAT,
                    root_id,
                    1,
                    MPI_COMM_WORLD,
                    &request));

                ///////////////////////////////////////////////
                // Do some other computations here if needed //
                ///////////////////////////////////////////////

                MPI_Error_Check(MPI_Wait(&request, &status));
            }          

            else if ( world_rank == root_id)
            {
                for (int i = 0; i < world_size - 1; i++)
                {
                    MPI_Error_Check(MPI_Probe(MPI_ANY_SOURCE,
                          1,
                          MPI_COMM_WORLD,
                          &status));

                    int source_rank = status.MPI_SOURCE;

                    if (source_rank == world_rank - 1)
                    {
                        processor_workload = processor_workload_last;
                    }

                    MPI_Error_Check(MPI_Irecv(J_processor.data(),
                          processor_workload,
                          MPI_FLOAT,
                          source_rank,
                          1,
                          MPI_COMM_WORLD,
                          &request));

                    ///////////////////////////////////////////////
                    // Do some other computations here if needed //
                    ///////////////////////////////////////////////

                    // Blocks and waits for destination process to receive data
                    MPI_Error_Check(MPI_Wait(&request, &status));

                    // Tests for completion of send or receive, itself is non-blocking
                    // MPI_Test()

                    // Waits for all given communications to complete
                    // MPI_Waitall()

                    Eigen::Map<Eigen::VectorXf> J_merged(J_processor.data(), processor_workload);

                    float recv_max_diff = (J.segment(source_rank * processor_workload, processor_workload) -
                                           J_merged).cwiseAbs().maxCoeff();

                    if (recv_max_diff > error)
                    {
                        error = recv_max_diff;
                    }

                    J.segment(source_rank * processor_workload, processor_workload) = J_merged;
                }
            }
                
            MPI_Error_Check(MPI_Ibcast(&error, 1, MPI_FLOAT, root_id, MPI_COMM_WORLD, &request));

            ///////////////////////////////////////////////
            // Do some other computations here if needed //
            ///////////////////////////////////////////////

            MPI_Error_Check(MPI_Wait(&request, &status));

            //MPI_Error_Check(MPI_Bcast(&error, 1, MPI_FLOAT, root_id, MPI_COMM_WORLD));

            // If convergence criteria is reached -> terminate
            if(error <= tolerance)
            {
                debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                break;
            }

            MPI_Error_Check(MPI_Ibcast(J.data(), J.size(), MPI_FLOAT, root_id, MPI_COMM_WORLD, &request));

            ///////////////////////////////////////////////
            // Do some other computations here if needed //
            ///////////////////////////////////////////////

            MPI_Error_Check(MPI_Wait(&request, &status));

            error = 0;
        }
    }



    std::vector<int> recvcounts;
    std::vector<int> displs;

    for(int i=0; i < world_size; ++i)
    {
        if(i+1 < world_size)
            recvcounts.push_back(processor_workload);
        else
            recvcounts.push_back(processor_workload_last);

        if(i == 0)
            displs.push_back(0);
        else
            displs.push_back(displs[i-1] + recvcounts[i-1]);
    }

    int* Pi_raw = Pi.data();
    MPI_Status status_gather;
    MPI_Request request_gather;

    MPI_Igatherv(&Pi_raw[processor_start],
            processor_workload,
            MPI_INT,
            Pi_raw,
            recvcounts.data(),
            displs.data(),
            MPI_INT,
            root_id,
            MPI_COMM_WORLD,
            &request_gather);

    // Do some other work here if needed

    MPI_Wait(&request_gather, &status_gather);
}

std::string VI_Processor_Impl_Distr_04::GetName()
{
    return VI_Processor_Base::GetName() + "-" + std::to_string(comm_period);
}