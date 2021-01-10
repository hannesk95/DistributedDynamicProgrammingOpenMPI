#include "vi_processor_impl_local.h"
#include <limits>

void VI_Processor_Impl_Local::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int u_max, 
        const unsigned int s_max, 
        const unsigned int f_max,
        const unsigned int T
    )
{
    const unsigned int S_max = (s_max*s_max)*f_max;

    for(unsigned int t=0; t < T; ++t)
    {
        float error = std::numeric_limits<float>::min();

        for(unsigned int s=0; s < S_max; ++s)
        {
            // Problem specific values
            unsigned int f = s / (s_max*s_max);
            unsigned int s_g = s % (s_max*s_max) / s_max;
            unsigned int s_i = s % (s_max*s_max) % s_max;

            // Slice rows from P
            const auto& P_s = P.innerVectors(s*u_max, u_max);

            // Storage for optimal action and corresponding cost
            int u_min = 0;
            float g_min = std::numeric_limits<float>::max();
                
            // u is action
            for(int u=0; u < P_s.outerSize(); ++u)
            {
                // Iterate over columns of current row
                Eigen::InnerIterator<typeof(P_s)> it(P_s, u);
                if(it)
                {
                    // Storage for action cost
                    float g_u = 0;

                    for(;;)
                    {
                        // If action is not defined for current state we will not end up in this loop!

                        // Cost for current action in current state
                        float g = 0;
                        if(s_g == s_i && u == 0) g = -100;
                        else if(f == 0)          g = 100;
                        else if(u != 0)          g = 5;
                        else                     g = 0;

                        // Accumulate possible actoin costs
                        g_u += it.value() * (g + alpha*J[it.col()]);

                        // Next value
                        if(!(++it))
                        {
                            // Reached end of row
                                
                            // Check if current action is optimal
                            if(g_u < g_min)
                            {
                                g_min = g_u;
                                u_min = u;
                            }
                            break;
                        }
                            
                    }
                }
            }

            // Calculate error to check for divergence
            float err_tilde = std::abs(J[s] - g_min);
            if(err_tilde > error) error = err_tilde;

            // Store new value and policy for current state
            Pi[s] = u_min;
            J[s] = g_min;
        }

        if(error <= e_max)
        {
            debug_message("Converged after " + std::to_string(t) + " iterations.");
            break;
        } 

    }
}