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

    if (world_size < 2)
    {
        std::cerr << "World size must be greater than 1 for " << argv[0] << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int processor_workload = ceil(J.size() / world_size);
    int processor_workload_last = processor_workload + (J.size() % world_size);
    int processor_start = processor_workload * world_rank;
    int processor_end = processor_workload * (world_rank +1);

    std::vector<float> J_new(J.size());
    auto J_temp = J;
    MPI_Request request;
    MPI_Status status;

    std::vector<int> recvcounts;
    std::vector<int> displs;

    for(int i=0; i < world_size; ++i)
    {
        if(i == world_size - 1)
        {
            recvcounts.push_back(J.size() % processor_workload);
            displs.push_back(displs[-1] + J.size() % processor_workload);
        }

        else
        {
            recvcounts.push_back(processor_workload);
            displs.push_back(i*processor_workload);
        }
    }




    float error = 0;

    for(unsigned int t=0; t < T; ++t)
    {

        // Perform value iteration step
        iteration_step(Pi, J, P, processor_start, processor_end);

        if(t % comm_period == 0)
        {
            if(world_rank == root_id)
            {
                float* J_raw = J.data();

                MPI_Igatherv(&J_raw[processor_start],
                            processor_workload,
                            MPI_FLOAT,
                            &J_new,
                            recvcounts.data(),
                            displs.data(),
                            MPI_FLOAT,
                            root_id,
                            MPI_COMM_WORLD,
                            &request);
            }

            Eigen::Map<Eigen::VectorXf> J_final(J_new.data(), J.size());

            auto deviation = (J_temp-J_final).cwiseAbs().maxCoeff();

            if(deviation <= tolerance)
            {
                debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                break;
            }

            J_temp = J;
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

std::string VI_Processor_Impl_Distr_04::GetName()
{
    return VI_Processor_Base::GetName() + "-" + std::to_string(comm_period);
}