# `src`

contains all `src` files for project

<br/><br/>

-------

## <a href='Async_VI.cpp' target='_blank'>`Async_VI.cpp`</a>

C++ backend. Needs complete change for other tasks than `Async VI`. 

Given functionality:
- `void state_to_tuple()` - casts state to tuple with fuel, goal star and current star
- `void one_step_cost` - calculates cost for given state and action (=control)
- `vec one_step_lookahead()` - Helper function to calculate the value for all action in a given state (`vec` is a `typedef` for `Eigen::VectorXd`)
- `void async_vi()` - same as in the C++ version, but using the python --> performs the aynchronous value iteration or loads the given data arrays from the data directory (depending on the `recalc` parameter)
- `void async_vi()` - overloads `async_vi()` to initialize the data given by the `Python` Interface