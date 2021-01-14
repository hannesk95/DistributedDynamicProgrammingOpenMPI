#include "vi_processor_impl_local.h"
#include <limits>

void VI_Processor_Impl_Local::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    )
{
    const unsigned int S_max = P.cols();

    for(unsigned int t=0; t < max_iter; ++t)
    {
        // Compute one value iteration step over all states
        float error = iteration_step(Pi, J, P, 0, S_max);

        if(error <= tolerance)
        {
            debug_message("Converged after " + std::to_string(t) + " iterations");
            break;
        } 

    }
}