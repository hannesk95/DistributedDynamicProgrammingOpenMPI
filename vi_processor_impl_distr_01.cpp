#include "vi_processor_impl_distr_01.h"
#include <limits>


void VI_Processor_Impl_Distr_01::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int u_max, 
        const unsigned int s_max, 
        const unsigned int f_max,
        const unsigned int T
    )
{
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    // Buffer for gather 
    std::vector<float> J_recv(J.size());

    // Split up states for each process (last process gets additional remainder)
    const int process_first_state = (J.size() / world_size) * world_rank;
    const int process_last_state =  (world_size == world_rank + 1) ? J.size() : (J.size() / world_size) * (world_rank + 1);

    std::vector<int> recvcounts;    // Number of states for each process
    std::vector<int> displs;        // Displacement of sub vectors for each process
    for(int i=0; i < world_size; ++i)
    {
        if(i+1 < world_size) recvcounts.push_back((J.size() / world_size));
        else recvcounts.push_back((J.size() / world_size) + J.size() % world_size);

        if(i == 0) displs.push_back(0);
        else displs.push_back(displs[i-1] + recvcounts[i-1]);
    }


    for(unsigned int t=0; t < T; ++t)
    {

        for(unsigned int s=process_first_state; s < process_last_state; ++s)
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

            // Store new value and policy for current state
            Pi[s] = u_min;
            J[s] = g_min;                    

        }

        if(t % comm_period == 0)
        {
            // Merge local updates together
            float* J_raw = J.data();
            MPI_Allgatherv(&J_raw[process_first_state], 
                        process_last_state - process_first_state,
                        MPI_FLOAT, 
                        J_recv.data(), 
                        recvcounts.data(), 
                        displs.data(), 
                        MPI_FLOAT, 
                        MPI_COMM_WORLD
            );

            // Check for convergence
            Eigen::Map<Eigen::VectorXf> J_new(J_recv.data(), J_recv.size());
            auto J_abs_diff = (J - J_new).cwiseAbs();
            auto max_diff = J_abs_diff.maxCoeff();
            float error = 0;

            MPI_Allreduce(&max_diff, &error, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

            // Overwrite J
            J = J_new;

            // If convergence rate is below threshold stop
            if(error <= e_max)
            {
                debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                break;
            } 
        }

    }

    // Merge Policy
    int* Pi_raw = Pi.data();
    MPI_Gatherv(&Pi_raw[process_first_state], 
                process_last_state - process_first_state,
                MPI_INT, 
                Pi_raw, 
                recvcounts.data(), 
                displs.data(), 
                MPI_INT, 
                0, 
                MPI_COMM_WORLD
    );
}


bool VI_Processor_Impl_Distr_01::HasResult()
{
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    return 0 == world_rank;
}