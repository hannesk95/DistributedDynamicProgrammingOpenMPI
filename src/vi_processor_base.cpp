#include "vi_processor_base.h"
#include "cnpy.h"
#include <mpi.h>
#include <omp.h>


VI_Processor_Base::VI_Processor_Base(const vi_processor_args_t& args, const int _root_id, const float _alpha, const float _tolerance)
    : alpha(_alpha), tolerance(_tolerance), root_id(_root_id)
{
    // Load Parameters
    cnpy::NpyArray raw_fuel_capacity = cnpy::npz_load(args.Param_npz_dict_filename, "fuel_capacity");
    cnpy::NpyArray raw_number_stars = cnpy::npz_load(args.Param_npz_dict_filename, "number_stars");
    cnpy::NpyArray raw_max_controls = cnpy::npz_load(args.Param_npz_dict_filename, "max_controls");
    cnpy::NpyArray raw_max_jump_range = cnpy::npz_load(args.Param_npz_dict_filename, "max_jump_range");
    cnpy::NpyArray raw_confusion_distance = cnpy::npz_load(args.Param_npz_dict_filename, "confusion_distance");
    cnpy::NpyArray raw_NS = cnpy::npz_load(args.Param_npz_dict_filename, "NS");

    // convert the content
    n_stars = raw_number_stars.as_vec<int>()[0];
    n_states = raw_max_controls.as_vec<int>()[0];

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


void VI_Processor_Base::Process(std::vector<int>& Pi_out, std::vector<float>& J_out, const unsigned int max_iter)
{
    J.get()->fill(0);
    Pi.get()->fill(0);

    value_iteration_impl(*Pi.get(), *J.get(), *P.get(), max_iter);

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

//    #pragma omp parallel for
    for(unsigned int state = process_first_state; state < process_last_state; ++state)
    {

        // Problem specific values
        unsigned int fuel = state / (n_stars*n_stars);
        unsigned int goal_star = state % (n_stars*n_stars) / n_stars;
        unsigned int current_star = state % (n_stars*n_stars) % n_stars;

        // Slice rows from P
        const auto& P_slice = P.innerVectors(state*n_states, n_states);

        // Storage for optimal action and corresponding cost
        int optimal_action = 0;
        float optimal_cost = std::numeric_limits<float>::max();
                    
        // u is action
//        #pragma omp parallel for
        for(int action = 0; action < P_slice.outerSize(); ++action)
        {
            // Iterate over columns of current row
            Eigen::InnerIterator<typeof(P_slice)> _iterator(P_slice, action);
            if(_iterator)
            {
                // Storage for action cost
                float cost_action = 0;

                for(;;)
                {
                    // If action is not defined for current state we will not end up in this loop!

                    // Cost for current action in current state
                    float cost = 0;
                    if(goal_star == current_star && action == 0) cost = -100;
                    else if(fuel == 0)          cost = 100;
                    else if(action != 0)          cost = 5;

                    // Accumulate possible actoin costs
                    cost_action += _iterator.value() * (cost + alpha*J[_iterator.col()]);

                    // Next value
                    if(!(++_iterator))
                    {
                        // Reached end of row
                                    
                        // Check if current action is optimal
                        if(cost_action < optimal_cost)
                        {
                            optimal_cost = cost_action;
                            optimal_action = action;
                        }
                        break;
                    }
                                
                }
            }
        }

        // Calculate error
        float new_error = std::abs(J[state] - optimal_cost);
        if(new_error > error) error = new_error;

        // Store new value and policy for current state
        Pi[state] = optimal_action;
        J[state] = optimal_cost;                    

    }

    return error;
}