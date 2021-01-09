#include <iostream>
#include <chrono>
#include <memory>
#include <mpi.h>

#include "vi_processor_impl_local.h"
#include "vi_processor_impl_distr_01.h"

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    vi_processor_args_t args = {
        .P_npy_indptr_filename      = "../data_debug/P_indptr.npy",
        .P_npy_indices_filename     = "../data_debug/P_indices.npy",
        .P_npy_data_filename        = "../data_debug/P_data.npy",
        .P_npy_shape_filename       = "../data_debug/P_shape.npy",
        .Param_npz_dict_filename    = "../data_debug/parameters.npz"
    };

    std::vector<std::unique_ptr<VI_Processor_Base>> processors;
    processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_01(args)));
    processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Local(args)));

    const int n_runs = 20;

    Eigen::MatrixXf measurements(n_runs, processors.size());

    for(int i=0; i<n_runs; i++)
    {
        for(int j=0; j<processors.size(); j++)
        {
            auto t0 = std::chrono::system_clock::now();
            (processors[j].get())->Process();
            auto t1 = std::chrono::system_clock::now();

            float time_in_sec = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1e6;

            measurements.coeffRef(i,j) = time_in_sec;

            //std::unique_ptr<Eigen::VectorXi> Pi;
            //std::unique_ptr<Eigen::VectorXf> J;
            //(processors[i].get())->GetPiAndJ(Pi, J);
        }
    }

    if(0 == world_rank)
    {
        // Print mean execution time for each processor implementation
        Eigen::VectorXf t_mean =  measurements.colwise().mean();
        std::cout << t_mean << std::endl;
    }

    MPI_Finalize();
}