// needs respective changes for real implementation - depending on actual implementation of Asynchronous Value Iteration

// C++ implementation of asynchronous value iteration

// Imports
#include "Async_VI.h" // header
#include <iostream> // for printing
#include <cmath> // for math calculations
#include <tuple> // for tuples
#include <chrono> // for time measuring

namespace Backend
{
  /**
   * casts state to tuple with fuel, goal star and current star
   * @param tuple: pointer to tuple [std::tuple<int, int, int>&]
   * @param state: current state [integer&]
   * @param n_stars: number of stars [const unsigned integer&]
   */
  void state_to_tuple(std::tuple<int, int, int>& tuple, int& state, const unsigned int& n_stars)
  {
    std::get<0>(tuple) = state / (n_stars * n_stars);
    std::get<1>(tuple) = state % (n_stars * n_stars) / n_stars;
    std::get<2>(tuple) = state % (n_stars * n_stars) % n_stars;
  }

  /**
   * calculates cost for given state and action (=control)
   * @param cost: pointer to costs [float&]
   * @param state: current state [integer&]
   * @param control: desired action to take [integer&]
   * @param n_stars: number of stars given [const unsigned integer&]
   */
  void one_step_cost(float& cost, int& state, int& control, const unsigned int& n_stars)
  {
    // init fuel, goal star and current star
    int f, g, i;
    std::tuple<int, int, int> tuple;
    // calculate fuel, goal star and current star from state
    state_to_tuple(tuple, state, n_stars);
    std::tie(f, g, i) = tuple;
    // in goal and no jump
    if (g == i && control == 0)
    {
        cost = -100.0;
    }
    // out of fuel
    else if (f == 0)
    {
        cost = 100.0;
    }
    // avoid unnecessary jumps
    else if (control > 0)
    {
        cost = 5.0;
    }
    else
    {
        // else no cost
        cost = 0.0;
    }
  }

  /**
   * Helper function to calculate the value for all action in a given state
   * @param probabilities: SparseMatrix with all probabilities for all states and actions [Eigen::Ref<SpMat>]
   * @param state: current state [Integer]
   * @param V: current values [Eigen::Ref<vec>]
   * @param nA: number of theoretical possible actions [const unsigned integer&]
   * @param n_stars: number of stars [const unsigned integer&]
   * @param alpha: discount factor - between 0 and 1, close to 1 [const float&]
   * @return A: cost array for all actions for this state [vec]
   */
  vec one_step_lookahead(Eigen::Ref<SpMat> probabilities, int state, Eigen::Ref<vec> V, const unsigned int& nA, const unsigned int& n_stars, const float& alpha)
  {
    // cut that piece of data from probability matrix that contains all actions for given state
    SpMat slice = probabilities.middleRows(state * nA, nA);
    // init number of possible actions
    int actions = 0;
    // go through all theoretical (nA) possible actions
    for (unsigned int i = 0; i < nA; i++)
    {
        // if any value in that row is bigger than 0, then this action actually exists
        if (slice.row(i).sum() > 0)
        {
            actions += 1;
        }
    }
    // init A as zero-array as long as actual possible actions for current state
    vec A(actions);
    A.fill(0.0);
    // make it parallel
    #pragma omp parallel for
    // iterate over non-zero elements of remaining piece of data
    for (int k = 0; k < slice.outerSize(); ++k)
    {
        for (SpMat::InnerIterator it(slice, k); it; ++it)
        {
            // get next_state and current action
            int next_state = it.col(); // == column index
            int action = it.row(); // == row index
            // calculate reward
            float reward;
            one_step_cost(reward, state, action, n_stars);
            // add respective costs to respective action
            A[action] += probabilities.coeffRef(state*nA + action, next_state) * (reward + alpha * V[next_state]);
        }
    }
    return A;
  }

  /**
   * Same as in the C++ version, but using python
   * @param V: value array [vec]
   * @param PI: policy array [vec]
   * @param probabilities: SparseMatrix with all probabilities for all states and actions [SpMat]
   * @param n_stars: number of stars [const unsigned integer]
   * @param nS: number of states [const unsigned integer]
   * @param nA: number of actions [const unsigned integer]
   */
  void async_vi(Eigen::Ref<vec> V, Eigen::Ref<vec> PI, Eigen::Ref<SpMat> probabilities, const unsigned int n_stars, const unsigned int nS, const unsigned int nA)
  {
    // start timer. to measure the time
    auto t0 = std::chrono::system_clock::now();
    // init number of threads
    omp_set_num_threads(4);
    // init tolerance, discount factor and delta
    const double tolerance = 1e-6;
    const float alpha = 0.99;
    double delta;
    // count the required epochs
    unsigned int epochs = 0;

    do
    {
        // reinit delta new for every epoch
        delta = 0;
        // make it parallel
        #pragma omp parallel for
        // go through all states
        for (unsigned int state = 0; state < nS; state++)
        {
            // calculate costs for this state
            vec A = one_step_lookahead(probabilities, state, V, nA, n_stars, alpha);
            // init index of minimal value in A
            vec::StorageIndex minIdx;
            // get minimal value of A
            float minOfA = A.minCoeff(&minIdx);
            // calculate delta of new value for this state with existing one
            delta = std::max(delta, std::abs(minOfA - V[state]));
            // set value and policy new
            V[state] = minOfA;
            PI[state] = minIdx;
        }
        epochs += 1;
    } while (delta >= tolerance);
    // measure the time needed for the algorithm and print that out
    auto t1 = std::chrono::system_clock::now();
    std::cout << "This took " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()/1e6 << " seconds and " << epochs << " iterations" << std::endl;
  }

  /**
   * overloads async_vi to initialize the data
   * @param V: double pointer of value array [double*]
   * @param PI: double pointer of policy array [double*]
   * @param values: double pointer of values of SparseMatrix [double*]
   * @param row_indices: int pointer of column indices of SparseMatrix [int*]
   * @param rowptr: int pointer of column pointers of SparseMatrix [int*]
   * @param nnz: number of non-zero elements of SparseMatrix [const unsigned integer]
   * @param cols: number of cols of SparseMatrix [const unsigned integer]
   * @param rows: number of rows of SparseMatrix [const unsigned integer]
   * @param n_stars: number of stars [const unsigned integer]
   * @param nS: number of states [const unsigned integer]
   * @param nA: number of actions [const unsigned integer]
   *
   * \overload
   */
  void async_vi(double* V, double* PI, double* values, int* row_indices, int* rowptr, const unsigned int nnz, const unsigned int cols, const unsigned int rows, const unsigned int n_stars, const unsigned int nS, const unsigned int nA)
  {
    // don't use a row major format here: the continuous data from outside comes from row major format
    // and must be written transposed in col major matrix
    // map V, PI and the probability matrix
    Eigen::Map<vec> v_map(V, nS);
    Eigen::Map<vec> pi_map(PI, nS);
    Eigen::Map<SpMat> probabilities_map(rows, cols, nnz, rowptr, row_indices, values);

    // run the actual value iteration
    async_vi(v_map, pi_map, probabilities_map, n_stars, nS, nA);
  }
}
