#include <iostream>
#include <chrono>
#include <memory>
#include <mpi.h>

#include "vi_processor_impl_local.h"
#include "vi_processor_impl_distr_01.h"
#include "vi_processor_impl_distr_02.h"

#include "cnpy.h"

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    std::string dataset = "data_debug";

    vi_processor_args_t args = {
        .P_npy_indptr_filename      = "../data/" + dataset + "/P_indptr.npy",
        .P_npy_indices_filename     = "../data/" + dataset + "/P_indices.npy",
        .P_npy_data_filename        = "../data/" + dataset + "/P_data.npy",
        .P_npy_shape_filename       = "../data/" + dataset + "/P_shape.npy",
        .Param_npz_dict_filename    = "../data/" + dataset + "/parameters.npz"
    };

    cnpy::NpyArray raw_J_star = cnpy::npy_load("../data/" + dataset + "/J_star_alpha_0_99_iter_1000.npy");
    cnpy::NpyArray raw_Pi_star = cnpy::npy_load("../data/" + dataset + "/pi_star_alpha_0_99_iter_1000.npy");

    std::vector<float> J_star_vec = raw_J_star.as_vec<float>();
    std::vector<int> Pi_star_vec = raw_Pi_star.as_vec<int>();

    Eigen::Map<Eigen::VectorXf> J_star(J_star_vec.data(), J_star_vec.size());
    Eigen::Map<Eigen::VectorXi> Pi_star(Pi_star_vec.data(), Pi_star_vec.size());


    std::vector<std::unique_ptr<VI_Processor_Base>> processors;
    for(const int& comm_period : {10,50,100,500})
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_01(args, 0, comm_period)));
    for(const int& comm_period : {10,50,100,500})
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_02(args, 0, comm_period)));
    processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Local(args, 0)));

    const int n_runs = 20;

    Eigen::MatrixXf measurements(n_runs, processors.size());
    Eigen::VectorXf mse_J(processors.size());
    Eigen::VectorXi err_Pi(processors.size());

    for(int i=0; i<n_runs; i++)
    {
        for(int j=0; j<processors.size(); j++)
        {
            std::vector<int> Pi_vec;
            std::vector<float> J_vec;

            auto t0 = std::chrono::system_clock::now();
            processors[j].get()->Process(Pi_vec, J_vec);
            auto t1 = std::chrono::system_clock::now();

            float time_in_sec = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1e6;
            measurements.coeffRef(i,j) = time_in_sec;

            Eigen::Map<Eigen::VectorXf> J(J_vec.data(), J_vec.size());
            auto J_sq_err = (J - J_star).cwiseAbs2();
            mse_J.coeffRef(j) = J_sq_err.mean();

            Eigen::Map<Eigen::VectorXi> Pi(Pi_vec.data(), Pi_vec.size());
            Eigen::VectorXi Pi_diff = (Pi - Pi_star);
            err_Pi.coeffRef(j) = (Pi_diff.array() > 0).count();
        }
    }

    if(0 == world_rank)
    {
        // Print mean execution time for each processor implementation
        std::cout << "========= Mean execution times =========" << std::endl;
        Eigen::VectorXf t_mean =  measurements.colwise().mean();
        std::cout << t_mean << std::endl;

        // Print mean square error of J vector for each processor implementation
        std::cout << "========= MSE's for J vectors =========" << std::endl;
        std::cout << mse_J << std::endl;
        
        // Print number of wrong entries in Pi for each processor implementation
        std::cout << "========= Number of errors in Pi vectors =========" << std::endl;
        std::cout << err_Pi << std::endl;
    }

    MPI_Finalize();
}