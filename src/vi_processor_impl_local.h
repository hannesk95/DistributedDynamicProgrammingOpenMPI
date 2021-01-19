// Header of local implementation, same as in homework 2

// be sure class is defined
#ifndef __VI_PROCESSOR_IMPL_LOCAL_H
#define __VI_PROCESSOR_IMPL_LOCAL_H
// includes
#include "vi_processor_base.h" // parent class

/**
 * local implementation
 */
class VI_Processor_Impl_Local : public VI_Processor_Base
{

public:
    /**
     * Constructor of local implementation
     * @param vi_data_args_t: arguments dictionary with path to respective .npy files [const Dictionary reference]
     * @param root_id: id of root processor [const Integer]
     */
    VI_Processor_Impl_Local(const vi_data_args_t& args, const int root_id)
            : VI_Processor_Base(args, root_id)
    {}

    /**
     * get name of implementation
     * @return name: name of implementation [String]
     */
    std::string GetName() override;

protected:
    /**
     * actual implementation of inherited abstract function for the communication scheme
     * @param Pi: initialized Pi [Integer vector reference]
     * @param J: initialized J [Float vector reference]
     * @param P: probability matrix [const Eigen SparseMatrix<RowMajor> reference]
     * @param max_iter: maximum number of iterations before stopping anyway [const unsigned Integer]
     */
    void value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    ) override;
};
#endif