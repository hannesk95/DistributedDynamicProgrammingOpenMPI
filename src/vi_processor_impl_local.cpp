// local implementation, same as in homework 2

// includes
#include "vi_processor_impl_local.h" // header

/**
 * get name of implementation
 * @return name: name of implementation [String]
 */
std::string VI_Processor_Impl_Local::GetName()
{
    return VI_Processor_Base::GetName();
}

/**
 * actual implementation of inherited abstract function for the communication scheme
 * @param Pi_out: initialized Pi [Integer vector reference]
 * @param J_out: initialized J [Float vector reference]
 * @param P: probability matrix [const Eigen SparseMatrix<RowMajor> reference]
 * @param max_iter: maximum number of iterations before stopping anyway [const unsigned Integer]
 */
void VI_Processor_Impl_Local::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    )
{
    // we use all states in every step
    const unsigned int S_max = P.cols();

    for(unsigned int t=0; t < max_iter; ++t)
    {
        // Compute one value iteration step over all states
        float error = iteration_step(Pi, J, P, 0, S_max);
        // If convergence rate is below threshold stop
        if(error <= tolerance)
        {
            debug_message("Converged after " + std::to_string(t) + " iterations");
            break;
        } 

    }
}
