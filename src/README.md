# `src`

folder containing all libraries that are used by <a href="../main.cpp" target="_blank">`main.cpp`</a>

<br/><br/>

-------

## <a href="vi_processor_base.cpp" target="_blank">`vi_processor_base.cpp`</a>

Base Class for all following Implementations of the desired task. Responsible for parsing the data from the given dataset from `.npy` files into <a href='http://eigen.tuxfamily.org/index.php?title=Main_Page' target='_blank'>`Eigen`</a> vectors.

Given functionality:
- `VI_Processor_Base` - reads all files `.npy` that are needed for the value iteration, transforms them to `Eigen` vectors/SparseMatrix
- `void debug_message()` - prints given debugging messages for respective processor
- `void Process()` - function that calls `value_iteration_impl` of respective processor
- `float iteration_step()` - performs the value iteration for given amount of steps (before remerging whole `probability matrix`) on given subset of the `probability matrix`

<br/><br/>

-------

## <a href="vi_processor_base.h" target="_blank">`vi_processor_base.h`</a>

Header of <a href="vi_processor_base.cpp" target="_blank">`vi_processor_base.cpp`</a>

<br/><br/>

-------

## <a href="vi_processor_impl_distr_01.cpp" target="_blank">`vi_processor_impl_distr_01.cpp`</a>

first implementation of a communication scheme. Using `MPI_Allgatherv`,`MPI_Allreduce` and `MPI_Gatherv`. Waits for given amount of periods before re-gathering the updated `J`, inherits functionality from `VI_Processor_Base`.

Given functionality:
- `void value_iteration_impl()` - concrete implementation of communication scheme, doing the value iteration

<br/><br/>

-------

## <a href="vi_processor_impl_distr_01.h" target="_blank">`vi_processor_impl_distr_01.h`</a>

Header of <a href="vi_processor_impl_distr_01.cpp" target="_blank">`vi_processor_impl_distr_01.cpp`</a>

<br/><br/>

-------

## <a href="vi_processor_impl_distr_02.cpp" target="_blank">`vi_processor_impl_distr_02.cpp`</a>

second implementation of a communication scheme. Using `MPI_Send` and `MPI_Recv`. Waits for given amount of periods before re-gathering the updated `J`, inherits functionality from `VI_Processor_Base`. Logic of communication scheme is the same as in `VI_Processor_Impl_Distr_01`

Given functionality:
- `void value_iteration_impl()` - concrete implementation of communication scheme, doing the value iteration

<br/><br/>

-------

## <a href="vi_processor_impl_distr_02.h" target="_blank">`vi_processor_impl_distr_02.h`</a>

Header of <a href="vi_processor_impl_distr_02.h" target="_blank">`vi_processor_impl_distr_02.h`</a>

<br/><br/>

-------

## <a href="vi_processor_impl_distr_42.cpp" target="_blank">`vi_processor_impl_distr_42.cpp`</a>

third implementation of a communication scheme. Using `MPI_Sendrecv` and `MPI_Gatherv`. Updates `J` every period completely, inherits functionality from `VI_Processor_Base`. Works synchronely

Given functionality:
- `void value_iteration_impl()` - concrete implementation of communication scheme, doing the value iteration

<br/><br/>

-------

## <a href="vi_processor_impl_distr_42.h" target="_blank">`vi_processor_impl_distr_42.h`</a>

Header of <a href="vi_processor_impl_distr_42.h" target="_blank">`vi_processor_impl_distr_42.h`</a>

<br/><br/>

-------

## <a href="vi_processor_impl_local.cpp" target="_blank">`vi_processor_impl_local.cpp`</a>

local implementation of value iteration without any communication scheme or parallelization. Implemented to use as reference for performance comparisons. Updates `J` every period completely, inherits functionality from `VI_Processor_Base`. Works synchronely

Given functionality:
- `void value_iteration_impl()` - concrete implementation of value iteration

<br/><br/>

-------

## <a href="vi_processor_impl_local.h" target="_blank">`vi_processor_impl_local.h`</a>

Header of <a href="vi_processor_impl_local.cpp" target="_blank">`vi_processor_impl_local.cpp`</a>