#include <iostream>
#include <chrono>
#include <memory>
#include <mpi.h>

#include "vi_processor_impl_local.h"
#include "vi_processor_impl_distr_01.h"
#include "vi_processor_impl_distr_02.h"
#include "vi_processor_impl_distr_03.h"
#include "vi_processor_impl_distr_04.h"
#include "vi_processor_impl_distr_05.h"

#include "cnpy.h"

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    std::string data_folder = "../data/data_debug"; // Here can the data be found
    std::string result_folder = "../results";       // Here shall the evaluation results be stored
    std::vector<int> comm_periods{10,50,100,500};   // Communication periods which shall be evaluated
    const int n_runs = 10;                          // Number of evaluation runs

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    vi_data_args_t args = {
        .P_npy_indptr_filename      = data_folder + "/P_indptr.npy",
        .P_npy_indices_filename     = data_folder + "/P_indices.npy",
        .P_npy_data_filename        = data_folder + "/P_data.npy",
        .P_npy_shape_filename       = data_folder + "/P_shape.npy",
        .Param_npz_dict_filename    = data_folder + "/parameters.npz"
    };

    cnpy::NpyArray raw_J_star = cnpy::npy_load(data_folder + "/J_star_alpha_0_99_iter_1000.npy");
    cnpy::NpyArray raw_Pi_star = cnpy::npy_load(data_folder + "/pi_star_alpha_0_99_iter_1000.npy");

    std::vector<float> J_star_vec = raw_J_star.as_vec<float>();
    std::vector<int> Pi_star_vec = raw_Pi_star.as_vec<int>();

    Eigen::Map<Eigen::VectorXf> J_star(J_star_vec.data(), J_star_vec.size());
    Eigen::Map<Eigen::VectorXi> Pi_star(Pi_star_vec.data(), Pi_star_vec.size());

    std::vector<std::unique_ptr<VI_Processor_Base>> processors;
    for(const int& comm_period : comm_periods)
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_01(args, 0, comm_period)));
    for(const int& comm_period : comm_periods)
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_02(args, 0, comm_period)));
    for(const int& comm_period : comm_periods)
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_03(args, 0, comm_period)));
    for(const int& comm_period : comm_periods)
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_04(args, 0, comm_period)));
    for(const int& comm_period : comm_periods)
        processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Distr_05(args, 0, comm_period)));

    processors.push_back(std::unique_ptr<VI_Processor_Base>(new VI_Processor_Impl_Local(args, 0)));

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
            Eigen::VectorXi Pi_diff = (Pi - Pi_star).cwiseAbs();
            err_Pi.coeffRef(j) = Pi_diff.sum();
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

        // save results
        int j = 0;
        for (auto& process : processors)
        {
            std::cout << process->GetName() << std::endl;
            auto parameters = process->GetParameters();
            for(auto &param:parameters)
            {
                std::cout << "\t" << param.first << ": " << param.second << std::endl;
            }
            cnpy::npz_save(result_folder + "/" + process->GetName() + ".npz", "mean_execution_time", t_mean.data() + j, {1}, "w");
            cnpy::npz_save(result_folder + "/" + process->GetName() + ".npz", "MSE_J", mse_J.data() + j, {1}, "a");
            cnpy::npz_save(result_folder + "/" + process->GetName() + ".npz", "errors_Pi", err_Pi.data() + j, {1}, "a");
            j += 1;
        }
    }

    MPI_Finalize();
}
