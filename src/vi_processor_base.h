#ifndef __VI_PROCESSOR_BASE_H
#define __VI_PROCESSOR_BASE_H


#define EIGEN_USE_BLAS
#define EIGEN_USE_LAPACKE
#include "Eigen/Sparse"

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

    // Id of processor which shall provide the results
    const int root_id;

    ///
    /// \brief Value iteration implementation
    /// 
    /// \param Pi           Optimal policy will be stored in this vector
    /// \param J            Value for each state will be stored in this vector
    /// \param P            Transition probability matrix
    /// \param T            Maximal number of iteration steps
    ///
    virtual void value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int T
    ) = 0;

    
    ///
    /// \brief Prints out a message if VI_PROCESSOR_DEBUG is defined
    /// 
    /// \param msg 
    ///
    void debug_message(std::string msg);


    const float alpha; // Discount factor
    const float e_max; // Convergence limit    

    ///
    /// \brief 
    /// 
    /// \param Pi                   Optimal policy will be stored in this vector
    /// \param J                    Value for each state will be stored in this vector
    /// \param P                    Transition probability matrix
    /// \param process_first_state  The first state in the range to update
    /// \param process_last_state   The last state in the range to update
    /// \return                     The maximum absolute change of cost values
    ///
    float iteration_step(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const int process_first_state, 
        const int process_last_state);

    private:    

    unsigned int u_max; // Number of possible actions
    unsigned int s_max; // Number of stars in navigation task
    unsigned int f_max; // Maximal fuel level

    std::vector<int> P_indptr;
    std::vector<int> P_indices;
    std::vector<float> P_data;
    std::vector<int64_t> P_shape;

    std::unique_ptr<Eigen::Map<SpMat_t>> P;
    std::unique_ptr<Eigen::VectorXf> J;
    std::unique_ptr<Eigen::VectorXi> Pi;

    public:

    VI_Processor_Base(const vi_processor_args_t& args, const int _root_id, const float _alpha = 0.99, const float _e_max = 1e-10);

    ///
    /// \brief Process the data
    /// 
    /// \param Pi_out Vector in which optimal strategy for each state shall be stored (only usefull in root processor)
    /// \param J_out  Vector in which optimal cost for each state shall be stored (only usefull in root processor)
    /// \param T      Maximal number of iteration steps
    ///
    void Process(std::vector<int>& Pi_out, std::vector<float>& J_out, const unsigned int T = 10e6);
    
    ///
    /// \brief Get the Name of the Processor
    /// 
    /// \return string 
    ///
    virtual std::string GetName();
};


#endif