// Base class for all implementations

// includes
#include "vi_processor_base.h" // header
#include "cnpy.h" // read .npy files
#include <mpi.h> // openmpi support

/**
 * Constructor of Base class for all implementations
 * @param args: arguments dictionary with path to respective .npy files [const Dictionary reference]
 * @param _root_id: id of root processor [const Integer]
 * @param _alpha: discount factor for value iteration [const Float]
 * @param _tolerance: tolerance for break condition [const Float]
 */
VI_Processor_Base::VI_Processor_Base(const vi_data_args_t& _args, const int _root_id, const float _alpha, const float _tolerance)
    : alpha(_alpha), tolerance(_tolerance), root_id(_root_id), args(_args)
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
}

/**
 * processes the value iteration. Gets called in main for every implementation
 * @param Pi_out: initialized Pi [Integer vector reference]
 * @param J_out: initialized J [Float vector reference]
 * @param max_iter: maximum number of iterations before stopping anyway [const unsigned Integer]
 */
void VI_Processor_Base::Process(std::vector<int>& Pi_out, std::vector<float>& J_out, const unsigned int max_iter)
{

    // Load Matrix
    cnpy::NpyArray raw_indptr = cnpy::npy_load(args.P_npy_indptr_filename);
    cnpy::NpyArray raw_indices = cnpy::npy_load(args.P_npy_indices_filename);
    cnpy::NpyArray raw_data = cnpy::npy_load(args.P_npy_data_filename);
    cnpy::NpyArray raw_shape = cnpy::npy_load(args.P_npy_shape_filename);

    // convert the content
    auto P_indptr = raw_indptr.as_vec<int>();
    auto P_indices = raw_indices.as_vec<int>();
    auto P_data = raw_data.as_vec<float>();
    auto P_shape = raw_shape.as_vec<int64_t>();

    // init SparseMatrix 
    auto P = Eigen::Map<SpMat_t>(P_shape[0], P_shape[1], P_data.size(), P_indptr.data(), P_indices.data(), P_data.data());
    auto J = Eigen::VectorXf(P_shape[1]);
    auto Pi = Eigen::VectorXi(P_shape[1]);

    // fill initialized J and Pi (from constructor) with 0s
    J.fill(0);
    Pi.fill(0);
    // do actual value iteration
    value_iteration_impl(Pi, J, P, max_iter);
    // set params to be calculated values
    Pi_out.assign(Pi.data(), Pi.data() + Pi.size());
    J_out.assign(J.data(), J.data() + J.size());
}

/**
 * get name of implementation
 * @return name: name of implementation [String]
 */
std::string VI_Processor_Base::GetName()
{
    return typeid(*this).name();
}

/**
 * set parameter of implementation
 * @param param: param name to set [String]
 * @param value: value of param to set [String]
 * @return hasParam: whether implementation has parameters [=true] or not [=false] [Boolean]
 */
bool VI_Processor_Base::SetParameter(std::string param, float value)
{
    if(param == "alpha")
    {
        alpha = value;
        return true;
    }
    if(param == "tolerance")
    {
        tolerance = value;
        return true;
    }
    return false;
}

/**
 * get parameter of implementation
 * @return parameters: mapped pair of "alpha" and "tolerance" [Pair]
 */
std::map<std::string, float> VI_Processor_Base::GetParameters()
{
    std::map<std::string, float> parameters;
    parameters["alpha"] = alpha;
    parameters["tolerance"] = tolerance;
    return parameters;
}

/**
 * performs the actual iteration step (for given amount of epochs before returning the result)
 * @param Pi: current Pi [Eigen Integer vector reference]
 * @param J: current J [Eigen Float vector reference]
 * @param P: probability matrix [const Eigen SparseMatrix<RowMajor> reference]
 * @param process_first_state: first state that shall be processed in sub part of whole state space [const Integer]
 * @param process_last_state: last state that shall be processed in sub part of whole state space [const Integer]
 * @return error: calculated difference between old and new J [Float]
 */
float VI_Processor_Base::iteration_step(
    Eigen::Ref<Eigen::VectorXi> Pi, 
    Eigen::Ref<Eigen::VectorXf> J, 
    const Eigen::Ref<const SpMat_t> P, 
    const int process_first_state, 
    const int process_last_state)
{
    // set error to 0
    float error = 0;
    // go through all states that shall be processed (not all for most implementations)
    for(unsigned int state = process_first_state; state < process_last_state; ++state)
    {
        // Problem specific values for fuel, goal star and current star
        unsigned int fuel = state / (n_stars*n_stars);
        unsigned int goal_star = state % (n_stars*n_stars) / n_stars;
        unsigned int current_star = state % (n_stars*n_stars) % n_stars;

        // Slice rows from P
        const auto& P_slice = P.innerVectors(state*n_states, n_states);

        // Storage for optimal action and corresponding cost
        int optimal_action = 0;
        float optimal_cost = std::numeric_limits<float>::max();
                    
        // go through all possible actions in that state
        for(int action = 0; action < P_slice.outerSize(); ++action)
        {
            // Iterate over columns of current row
            Eigen::InnerIterator<typeof(P_slice)> _iterator(P_slice, action);
            if(_iterator) // if value is not 0
            {
                // Storage for action cost
                float cost_action = 0;

                for(;;) // If action is not defined for current state we will not end up in this loop!
                {
                    // Cost for current action in current state
                    float cost = 0;
                    if(goal_star == current_star && action == 0) cost = -100;
                    else if(fuel == 0)          cost = 100;
                    else if(action != 0)          cost = 5;

                    // Accumulate possible action costs
                    cost_action += _iterator.value() * (cost + alpha*J[_iterator.col()]);

                    // Next value
                    if(!(++_iterator)) // Reached end of row
                    {
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

/**
 * debugging message to print some output onto the screen
 * @param msg: message to print [String]
 */
void VI_Processor_Base::debug_message(std::string msg)
{
#ifdef VI_PROCESSOR_DEBUG
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if(root_id == world_rank) // only 'master' prints, not every processor
    {
        std::cout << "[" << GetName() << "]: " << msg << std::endl;
    }
#endif
}
