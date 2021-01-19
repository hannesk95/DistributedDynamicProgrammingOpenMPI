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

    MPI_Status status;
    MPI_Request request;

    int processor_workload = ceil(float(J.size()) / world_size);
    //int processor_workload_last = J.size() % processor_workload;
    int processor_workload_last = J.size() - (world_size - 1)*processor_workload;

    int processor_start = processor_workload * world_rank;
    int processor_end = processor_workload * (world_rank +1);

    //std::cout << processor_workload << std::endl;
    //std::cout << processor_workload_last << std::endl;
    //std::cout << processor_workload + processor_workload_last << std::endl;


    if (world_rank == world_size -1)
    {
        processor_end = J.size();
    }

    std::vector<float> J_buffer(processor_workload);
    std::vector<float> Pi_buffer(processor_workload);

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
            if ( world_rank != root_id)
            {

                int send_workload = processor_workload;

                if( world_rank == world_size - 1)
                {
                    send_workload = processor_workload_last;
                }

                float* J_raw = J.data();

                MPI_Isend(J_raw + processor_start,
                        send_workload,
                        MPI_FLOAT,
                        root_id,
                        1,
                        MPI_COMM_WORLD,
                        &request);
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
                    int recv_workload = processor_workload;

                    if (source_rank == world_size - 1)
                    {
                        recv_workload = processor_workload_last;
                        //J_buffer.resize(processor_workload_last);
                        //Pi_buffer.resize(processor_workload_last);
                    }

                    MPI_Irecv(J_buffer.data(),
                          recv_workload,
                          MPI_FLOAT,
                          source_rank,
                          1,
                          MPI_COMM_WORLD,
                          &request);
                    ///////////////////////////////////////////////
                    // Do some other computations here if needed //
                    ///////////////////////////////////////////////
                    MPI_Wait(&request, &status);

                    Eigen::Map<Eigen::VectorXf> J_sub(J_buffer.data(), recv_workload);

                    auto number = J.segment(source_rank * processor_workload, recv_workload);

                    float recv_max_diff = (number - J_sub).cwiseAbs().maxCoeff();

                    if (recv_max_diff > error)
                    {
                        error = recv_max_diff;
                    }

                    J.segment(source_rank * processor_workload, recv_workload) = J_sub;

                    //MPI_Wait(&request, &status); // Blocks and waits for destination process to receive data


                }
            }
            else throw std::runtime_error("Something strange happened!");
                
            MPI_Ibcast(&error, 1, MPI_FLOAT, root_id, MPI_COMM_WORLD, &request);
            ///////////////////////////////////////////////
            // Do some other computations here if needed //
            ///////////////////////////////////////////////
            MPI_Wait(&request, &status);

            if(error <= tolerance)
            {
                debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                break;
            }

            MPI_Ibcast(J.data(), J.size(), MPI_FLOAT, root_id, MPI_COMM_WORLD, &request);
            ///////////////////////////////////////////////
            // Do some other computations here if needed //
            ///////////////////////////////////////////////
            MPI_Wait(&request, &status);

            error = 0;
        }
    }

    std::vector<int> recvcounts;
    std::vector<int> displs;



    for(int i=0; i < world_size; i++)
    {
        if(i == world_size - 1)
        {
            recvcounts.push_back(processor_workload_last);
            displs.push_back(i*processor_workload);
        }

        else
        {
            recvcounts.push_back(processor_workload);
            displs.push_back(i*processor_workload);
        }
    }

    for(auto& el:recvcounts){std::cout << el << std::endl;}
    for(auto& el:displs){std::cout << el << std::endl;}


    int* Pi_raw = Pi.data();

    MPI_Igatherv(&Pi_raw[processor_start],
            processor_end - processor_start,
            MPI_INT,
            Pi_raw,
            recvcounts.data(),
            displs.data(),
            MPI_INT,
            root_id,
            MPI_COMM_WORLD,
            &request);

    ///////////////////////////////////////////////
    // Do some other computations here if needed //
    ///////////////////////////////////////////////

    MPI_Wait(&request, &status);
}

std::string VI_Processor_Impl_Distr_04::GetName()
{
    return VI_Processor_Base::GetName() + "-" + std::to_string(comm_period);
}