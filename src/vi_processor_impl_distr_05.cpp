#include "vi_processor_impl_distr_05.h"
#include <limits>
#include <mpi.h>
#include <math.h>

#define MPI_Error_Check(x) {const int err=x; if(x!=MPI_SUCCESS) { fprintf(stderr, "MPI ERROR %d at %d.", err, __LINE__);}}


bool VI_Processor_Impl_Distr_05::SetParameter(std::string param, float value)
{
    if(param == "comm_period")
    {
        comm_period = value;
        return true;
    }

    return VI_Processor_Base::SetParameter(param, value);
}

std::map<std::string, float> VI_Processor_Impl_Distr_05::GetParameters()
{
    std::map<std::string, float> parameters = VI_Processor_Base::GetParameters();
    parameters["comm_period"] = comm_period;
    return parameters;
}


void VI_Processor_Impl_Distr_05::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi,
        Eigen::Ref<Eigen::VectorXf> J,
        const Eigen::Ref<const SpMat_t> P,
        const unsigned int T)
{
    int world_size, world_rank;
    MPI_Error_Check(MPI_Comm_size(MPI_COMM_WORLD, &world_size));
    MPI_Error_Check(MPI_Comm_rank(MPI_COMM_WORLD, &world_rank));

//    if (world_size < 2)
//    {
//        std::cerr << "World size must be greater than 1 otherwise OpenMPI makes no sense!" << std::endl;
//        MPI_Abort(MPI_COMM_WORLD, 1);
//    }

    int processor_workload = ceil(J.size() / world_size);
    int processor_start = processor_workload * world_rank;
    int processor_end = processor_workload * (world_rank +1);

    std::vector<float> J_new(J.size());
    std::vector<float> J_temp(0);
    Eigen::VectorXf J_tempp(J.size());
    J_tempp = J.segment(0, J.size());

    for(int i = 0; i < J.size(); i++)
    {
        J_temp.push_back(J[i]);
    }

    MPI_Request request;
    MPI_Status status;

    std::vector<int> recvcounts;
    std::vector<int> displs;

    for(int i=0; i < world_size; ++i)
    {
        if(i == world_size - 1)
        {
            recvcounts.push_back(J.size() % processor_workload);
            displs.push_back(i*processor_workload);
        }

        else
        {
            recvcounts.push_back(processor_workload);
            displs.push_back(i*processor_workload);
        }
    }

    for(unsigned int t=0; t < T; ++t)
    {
        iteration_step(Pi, J, P, processor_start, processor_end);

        if(t % comm_period == 0)
        {
            float* J_raw = J.data();

            if(world_rank == root_id)
            {
                MPI_Igatherv(&J_raw[processor_start],
                            processor_workload,
                            MPI_FLOAT,
                            J_raw,
                            recvcounts.data(),
                            displs.data(),
                            MPI_FLOAT,
                            root_id,
                            MPI_COMM_WORLD,
                            &request);

                Eigen::Map<Eigen::VectorXf> J_final(J_raw, J.size());
                //Eigen::Map<Eigen::VectorXf> J_store(J_temp.data(), J.size());

                auto deviation = (J_tempp-J_final).cwiseAbs().maxCoeff();

                if(deviation <= tolerance)
                {
                    debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                    break;
                }

                //J_temp.resize(0);

                J_tempp = J.segment(0, J.size());

//                for(int i = 0; i < J.size(); i++)
//                {
//                    J_temp.push_back(J[i]);
//                }
            }
        }
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

std::string VI_Processor_Impl_Distr_05::GetName()
{
    return VI_Processor_Base::GetName() + "-" + std::to_string(comm_period);
}