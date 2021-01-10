#ifndef __VI_PROCESSOR_BASE_H
#define __VI_PROCESSOR_BASE_H


#define EIGEN_USE_BLAS
#define EIGEN_USE_LAPACKE
#include "Eigen/Sparse"

#include "cnpy.h"

#include <mpi.h>
#include <string>
#include <memory>
#include <iostream>

#define VI_PROCESSOR_DEBUG 

///
/// \brief Value Iteration processor input arguments
/// 
///
typedef struct {
    std::string P_npy_indptr_filename;
    std::string P_npy_indices_filename;
    std::string P_npy_data_filename;
    std::string P_npy_shape_filename;
    std::string Param_npz_dict_filename;
}vi_processor_args_t;

typedef Eigen::SparseMatrix<float, Eigen::RowMajor> SpMat_t;

class VI_Processor_Base {
    protected:

    ///
    /// \brief Value iteration implementation
    /// 
    /// \param Pi           Optimal policy will be stored in this vector
    /// \param J            Value for each state will be stored in this vector
    /// \param P            Transition probability matrix
    /// \param u_max        Number of possible actions
    /// \param s_max        Number of stars in navigation task
    /// \param f_max        Maximal fuel level
    /// \param T            Maximal number of iteration steps
    ///
    virtual void value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int u_max, 
        const unsigned int s_max, 
        const unsigned int f_max,
        const unsigned int T
    ) = 0;
    

    // Discount factor and convergence limit
    const float alpha;
    const float e_max;
    

    void debug_message(std::string msg)
    {
        
        #ifdef VI_PROCESSOR_DEBUG
        int world_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
        if(0 == world_rank)
        {
            std::cout << "[" << typeid(*this).name() << "]: " << msg << std::endl;
        }
        #endif
    }


    private:

    unsigned int u_max;
    unsigned int s_max;
    unsigned int f_max;

    std::vector<int> P_indptr;
    std::vector<int> P_indices;
    std::vector<float> P_data;
    std::vector<int64_t> P_shape;

    std::unique_ptr<Eigen::Map<SpMat_t>> P;
    std::unique_ptr<Eigen::VectorXf> J;
    std::unique_ptr<Eigen::VectorXi> Pi;

    public:

    VI_Processor_Base(const vi_processor_args_t& args, const float _alpha = 0.99, const float _e_max = 10e-12)
        : alpha(_alpha), e_max(_e_max)
    {
        // Load Parameters
        cnpy::NpyArray raw_fuel_capacity = cnpy::npz_load(args.Param_npz_dict_filename, "fuel_capacity");
        cnpy::NpyArray raw_number_stars = cnpy::npz_load(args.Param_npz_dict_filename, "number_stars");
        cnpy::NpyArray raw_max_controls = cnpy::npz_load(args.Param_npz_dict_filename, "max_controls");
        cnpy::NpyArray raw_max_jump_range = cnpy::npz_load(args.Param_npz_dict_filename, "max_jump_range");
        cnpy::NpyArray raw_confusion_distance = cnpy::npz_load(args.Param_npz_dict_filename, "confusion_distance");
        cnpy::NpyArray raw_NS = cnpy::npz_load(args.Param_npz_dict_filename, "NS");

        // convert the content
        f_max = raw_fuel_capacity.as_vec<int>()[0];
        s_max = raw_number_stars.as_vec<int>()[0];
        u_max = raw_max_controls.as_vec<int>()[0];

        // Load Matrix
        cnpy::NpyArray raw_indptr = cnpy::npy_load(args.P_npy_indptr_filename);
        cnpy::NpyArray raw_indices = cnpy::npy_load(args.P_npy_indices_filename);
        cnpy::NpyArray raw_data = cnpy::npy_load(args.P_npy_data_filename);
        cnpy::NpyArray raw_shape = cnpy::npy_load(args.P_npy_shape_filename);

        // convert the content
        P_indptr = std::move(raw_indptr.as_vec<int>());
        P_indices = std::move(raw_indices.as_vec<int>());
        P_data = std::move(raw_data.as_vec<float>());
        P_shape = std::move(raw_shape.as_vec<int64_t>());

        // init SparseMatrix (Use P.get() to get raw pointer)
        P = std::move(std::unique_ptr<Eigen::Map<SpMat_t>>(new Eigen::Map<SpMat_t>(P_shape[0], P_shape[1], P_data.size(), P_indptr.data(), P_indices.data(), P_data.data())));
        J = std::move(std::unique_ptr<Eigen::VectorXf>(new Eigen::VectorXf(P_shape[1])));
        Pi = std::move(std::unique_ptr<Eigen::VectorXi>(new Eigen::VectorXi(P_shape[1])));
    }

    ///
    /// \brief Process the data
    /// 
    /// \param T Maximal number of iterations
    ///
    void Process(const unsigned int T = 10e6)
    {
        J.get()->fill(0.0);
        value_iteration_impl(*Pi.get(), *J.get(), *P.get(), u_max, s_max, f_max, T);
    }

    ///
    /// \brief Check if this processor contains the result
    /// 
    /// \return true 
    /// \return false 
    ///
    virtual bool HasResult() = 0;

    ///
    /// \brief Get the Pi And J vector
    /// 
    /// \param Pi_out 
    /// \param J_out 
    ///
    void GetPiAndJ(std::unique_ptr<Eigen::VectorXi>& Pi_out, std::unique_ptr<Eigen::VectorXf>& J_out)
    {
        Pi_out = std::move(std::unique_ptr<Eigen::VectorXi>(new Eigen::VectorXi(*Pi.get())));
        J_out = std::move(std::unique_ptr<Eigen::VectorXf>(new Eigen::VectorXf(*J.get())));
    }
};


#endif