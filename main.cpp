#include <iostream>
#include <chrono>
#include <memory>
#include <mpi.h>

#include "vi_processor_impl_local.h"
#include "vi_processor_impl_distr_01.h"

#include "cnpy.h"

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

    cnpy::NpyArray raw_J_star = cnpy::npy_load("../data_debug/J_star_alpha_0_99_iter_1000.npy");
    cnpy::NpyArray raw_Pi_star = cnpy::npy_load("../data_debug/pi_star_alpha_0_99_iter_1000.npy");

    std::vector<float> J_star_vec = raw_J_star.as_vec<float>();
    std::vector<int> Pi_star_vec = raw_Pi_star.as_vec<int>();

    Eigen::Map<Eigen::VectorXf> J_star(J_star_vec.data(), J_star_vec.size());
    Eigen::Map<Eigen::VectorXi> Pi_star(Pi_star_vec.data(), Pi_star_vec.size());


    std::vector<std::unique_ptr<VI_Processor_Base>> processors;
    for(const int& comm_period : {10,50,100,500})
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_01(args, comm_period)));
    processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Local(args)));

    const int n_runs = 20;

    Eigen::MatrixXf measurements(n_runs, processors.size());
    Eigen::VectorXf mse_J(processors.size());

    for(int i=0; i<n_runs; i++)
    {
        for(int j=0; j<processors.size(); j++)
        {
            auto t0 = std::chrono::system_clock::now();
            (processors[j].get())->Process();
            auto t1 = std::chrono::system_clock::now();

            float time_in_sec = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1e6;

            measurements.coeffRef(i,j) = time_in_sec;

            std::unique_ptr<Eigen::VectorXi> Pi;
            std::unique_ptr<Eigen::VectorXf> J;
            (processors[i].get())->GetPiAndJ(Pi, J);

            auto J_se = (*J.get() - J_star).cwiseAbs2();
            mse_J.coeffRef(j) = J_se.mean();
        }
    }

    if(0 == world_rank)
    {
        // Print mean execution time for each processor implementation
        std::cout << "========= Mean execution times =========" << std::endl;
        Eigen::VectorXf t_mean =  measurements.colwise().mean();
        std::cout << t_mean << std::endl;

        
        // Print mean squared error of J for each processor implementation
        std::cout << "========= MSE's of J vectors =========" << std::endl;
        std::cout << mse_J << std::endl;
    }

    MPI_Finalize();
}