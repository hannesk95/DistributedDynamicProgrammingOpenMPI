#include "vi_processor_impl_distr_04.h"
#include <limits>
#include <mpi.h>
#include <math.h>

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
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    MPI_Status status_send;
    MPI_Request request_send;

    MPI_Status status_recv;
    MPI_Request request_recv;

    MPI_Status status_bcst;
    MPI_Request request_bcst;

    MPI_Status status_prb;
    MPI_Request request_prb;

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
            // All slaves (world_rank > 0)
            if ( world_rank != 0)
            {
                float* J_raw = J.data();

                MPI_Isend(J_raw + processor_start,
                    processor_workload,
                    MPI_FLOAT,
                    root_id,
                    1,
                    MPI_COMM_WORLD,
                    &request_send);

                MPI_Wait(&request_send, &status_send);
            }          

            // Only master (world_rank == 0)
            else
            {
                for (int i = 0; i < world_size - 1; i++) {


                    MPI_Probe(MPI_ANY_SOURCE,
                              1,
                              MPI_COMM_WORLD,
                              &status_prb);

                    int source_rank = status_prb.MPI_SOURCE;

                    if (source_rank == world_rank - 1) {
                        processor_workload = processor_workload_last;
                    }

                    MPI_Irecv(J_processor.data(),
                              processor_workload,
                              MPI_FLOAT,
                              source_rank,
                              1,
                              MPI_COMM_WORLD,
                              &request_recv);

                    Eigen::Map<Eigen::VectorXf> J_merged(J_processor.data(), processor_workload);

                    float recv_max_diff = (J.segment(source_rank * processor_workload, processor_workload) -
                                           J_merged).cwiseAbs().maxCoeff();

                    if (recv_max_diff > error) {
                        error = recv_max_diff;
                    }

                    J.segment(source_rank * processor_workload, processor_workload) = J_merged;

                    // Blocks and waits for destination process to receive data
                    MPI_Wait(&request_recv, &status_recv);
                }
            }
                
            MPI_Ibcast(&error, 1, MPI_FLOAT, root_id, MPI_COMM_WORLD, &request_bcst);

            // If convergence criteria is reached -> terminate
            if(error <= tolerance)
            {
                debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                break;
            }

            MPI_Wait(&request_bcst, &status_bcst);

            MPI_Ibcast(J.data(), J.size(), MPI_FLOAT, root_id, MPI_COMM_WORLD, &request_bcst);

            error = 0;
            MPI_Wait(&request_bcst, &status_bcst);
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

    MPI_Request request;

    int* Pi_raw = Pi.data();

    MPI_Igatherv(&Pi_raw[processor_start],
            processor_workload,
            MPI_INT,
            Pi_raw,
            recvcounts.data(),
            displs.data(),
            MPI_INT,
            root_id,
            MPI_COMM_WORLD,
            &request);


}

std::string VI_Processor_Impl_Distr_04::GetName()
{
    return VI_Processor_Base::GetName() + "-" + std::to_string(comm_period);
}