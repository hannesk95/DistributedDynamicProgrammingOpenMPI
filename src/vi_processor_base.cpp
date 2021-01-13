#include "vi_processor_base.h"
#include "cnpy.h"
#include <mpi.h>
#include <omp.h>


VI_Processor_Base::VI_Processor_Base(const vi_processor_args_t& args, const int _root_id, const float _alpha, const float _e_max)
    : alpha(_alpha), e_max(_e_max), root_id(_root_id)
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


void VI_Processor_Base::debug_message(std::string msg)
{
        
    #ifdef VI_PROCESSOR_DEBUG
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if(root_id == world_rank)
    {
        std::cout << "[" << typeid(*this).name() << "]: " << msg << std::endl;
    }
    #endif
}


void VI_Processor_Base::Process(std::vector<int>& Pi_out, std::vector<float>& J_out, const unsigned int T)
{
    J.get()->fill(0);
    Pi.get()->fill(0);

    value_iteration_impl(*Pi.get(), *J.get(), *P.get(), T);

    Pi_out.assign(Pi.get()->data(), Pi.get()->data() + Pi.get()->size());
    J_out.assign(J.get()->data(), J.get()->data() + J.get()->size());
}


float VI_Processor_Base::iteration_step(
    Eigen::Ref<Eigen::VectorXi> Pi, 
    Eigen::Ref<Eigen::VectorXf> J, 
    const Eigen::Ref<const SpMat_t> P, 
    const int process_first_state, 
    const int process_last_state)
{
    float error = 0;

    #pragma omp parallel for
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
        #pragma omp parallel for
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

        // Calculate error
        float err_tilde = std::abs(J[s] - g_min);
        if(err_tilde > error) error = err_tilde;

        // Store new value and policy for current state
        Pi[s] = u_min;
        J[s] = g_min;                    

    }

    return error;
}