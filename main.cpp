#include <iostream>
#include <chrono>
#include <mpi.h>

#include "cnpy.h"

#include "vi_processor_impl_local.h"
#include "vi_processor_impl_distr_01.h"

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);



    vi_processor_args_t args = {
        .P_npy_indptr_filename      = "../data_debug/P_indptr.npy",
        .P_npy_indices_filename     = "../data_debug/P_indices.npy",
        .P_npy_data_filename        = "../data_debug/P_data.npy",
        .P_npy_shape_filename       = "../data_debug/P_shape.npy",
        .Param_npz_dict_filename    = "../data_debug/parameters.npz"
    };


    VI_Processor_Impl_Distr_01 proc(args);
    proc.SetCommPeriod(10);

    auto t0 = std::chrono::system_clock::now();
    proc.Process();
    auto t1 = std::chrono::system_clock::now();

    if(proc.HasResult()) {
        std::cout
            << "Processing took "
            << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()
            << " micro seconds" << std::endl;


        std::unique_ptr<Eigen::VectorXi> Pi;
        std::unique_ptr<Eigen::VectorXf> J;

        proc.GetPiAndJ(Pi, J);

        std::cout << "J:" << std::endl;
        std::cout << *(J.get()) << std::endl;
        
        std::cout << "Pi:" << std::endl;
        std::cout << *(Pi.get()) << std::endl;
    }



    MPI_Finalize();
}