// Header of Base class for all implementations

// be sure class is defined
#ifndef __VI_PROCESSOR_BASE_H
#define __VI_PROCESSOR_BASE_H
// be sure Eigen uses BLAS and LAPACKE
#define EIGEN_USE_BLAS
#define EIGEN_USE_LAPACKE
#define VI_PROCESSOR_DEBUG
// includes
#include "Eigen/Sparse" // for 'numpy-similar' operations
#include <string> // string usage
#include <memory> // memory optimization
#include <map> // mapping functionality
#include <iostream> // printing

/**
 * helping typedef (=Dictionary) to store the file paths
 */
typedef struct {
    std::string P_npy_indptr_filename;
    std::string P_npy_indices_filename;
    std::string P_npy_data_filename;
    std::string P_npy_shape_filename;
    std::string Param_npz_dict_filename;
}vi_data_args_t;

/**
 * helping typedef for SparseMatrix, float, RowMajor
 */
typedef Eigen::SparseMatrix<float, Eigen::RowMajor> SpMat_t;

/**
 * Base class for all implementations
 */
class VI_Processor_Base {
public:
    /**
     * Constructor of Base class for all implementations
     * @param args: arguments dictionary with path to respective .npy files [const Dictionary reference]
     * @param _root_id: id of root processor [const Integer]
     * @param _alpha: discount factor for value iteration [const Float]
     * @param _tolerance: tolerance for break condition [const Float]
     */
    VI_Processor_Base(const vi_data_args_t& args, const int _root_id, const float _alpha = 0.99, const float _tolerance = 1e-10);

    /**
     * processes the value iteration. Gets called in main for every implementation
     * @param Pi_out: initialized Pi [Integer vector reference]
     * @param J_out: initialized J [Float vector reference]
     * @param max_iter: maximum number of iterations before stopping anyway [const unsigned Integer]
     */
    void Process(std::vector<int>& Pi_out, std::vector<float>& J_out, const unsigned int max_iter = 10e6);

    /**
     * get name of implementation
     * @return name: name of implementation [String]
     */
    virtual std::string GetName();

    /**
     * set parameter of implementation
     * @param param: param name to set [String]
     * @param value: value of param to set [String]
     * @return hasParam: whether implementation has parameters [=true] or not [=false] [Boolean]
     */
    virtual bool SetParameter(std::string param, float value);

    /**
     * get parameter of implementation
     * @return parameters: mapped pair of "alpha" and "tolerance" [Pair]
     */
    virtual std::map<std::string, float> GetParameters();

private:
    vi_data_args_t args; // Data arguments (file locations)

    unsigned int n_states; // Number of possible actions
    unsigned int n_stars; // Number of stars in navigation task

protected:
    const int root_id; // Id of processor which shall provide the results [Integer]
    float alpha; // Discount factor [Float]
    float tolerance; // Convergence limit [Float]

    /**
     * abstract function that gets overwritten by every actual implementation for the communication scheme
     * @param Pi: initialized Pi [Integer vector reference]
     * @param J: initialized J [Float vector reference]
     * @param P: probability matrix [const Eigen SparseMatrix<RowMajor> reference]
     * @param max_iter: maximum number of iterations before stopping anyway [const unsigned Integer]
     */
    virtual void value_iteration_impl(
            Eigen::Ref<Eigen::VectorXi> Pi,
            Eigen::Ref<Eigen::VectorXf> J,
            const Eigen::Ref<const SpMat_t> P,
            const unsigned int max_iter
    ) = 0;

    /**
     * get name of implementation
     * @return name: name of implementation [String]
     */
    void debug_message(std::string msg);

    /**
     * performs the actual iteration step (for given amount of epochs before returning the result)
     * @param Pi: current Pi [Eigen Integer vector reference]
     * @param J: current J [Eigen Float vector reference]
     * @param P: probability matrix [const Eigen SparseMatrix<RowMajor> reference]
     * @param process_first_state: first state that shall be processed in sub part of whole state space [const Integer]
     * @param process_last_state: last state that shall be processed in sub part of whole state space [const Integer]
     * @return error: calculated difference between old and new J [Float]
     */
    float iteration_step(
            Eigen::Ref<Eigen::VectorXi> Pi,
            Eigen::Ref<Eigen::VectorXf> J,
            const Eigen::Ref<const SpMat_t> P,
            const int process_first_state,
            const int process_last_state);
};


#endif