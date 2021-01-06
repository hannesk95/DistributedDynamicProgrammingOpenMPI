# Always specify which version you need:
# - at least VERSION 3
# - EIKON has VERSION 3.10.2 available
cmake_minimum_required (VERSION 3.10)

# Set the name for the case this CMakeLists.txt is used directly.
# A description of the variables is given in the top level CMakeLists
if(NOT DEFINED LIB_NAME)
  set(LIB_NAME backend)
  set(DEST_LIB "${PROJECT_SOURCE_DIR}/install/lib")
  set(DEST_INC "${PROJECT_SOURCE_DIR}/install/")  
endif()

# A the static / shared library for the Python Code
#
# Do not use this CMakeLists.txt directly, work with the file in the parent directory

# Additional 'find module' scipts in the subfolder 'cmake'
# Currently, FindLAPACKE is the important one. FindMKL exist for the case that you need it in another project.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/;${CMAKE_MODULE_PATH}")

# We want parallel code later-on ...
find_package(OpenMP REQUIRED)

# Basic Linear Algebra Subprogram (BLAS): description of fastest elementary matrix / vector operations
# The Fortran realization of BLAS is provided by 'OpenBlas', and should be the fastest implementation
# without vendor lock.
#
# Intel's Math Kerner Library (MKL) is normally the best choice for even faster linear algebra code,
# but as the name suggests it is not available on AMD Machines. Also our HPC CPUs are too old ...
#
# Capital letters required here and in the following, I learned it the hard way ...
find_package(BLAS REQUIRED)    

# The Linear Algebra Package (LAPACK) makes use of the operations defined in BLAS to provide the actual 
# linear algebra stuff (e.g. equation solver or SVD), Numpy also uses it in the background.
#
# OpenLapack provides the implementation similar to OpenBlas, Intel MKL would be faster and has everything
# included (BLAS + LAPACK + OpenMP), but Intel is greedy and restricts it to their CPUs ...
find_package(LAPACK REQUIRED)  

# Interface for Eigen between C++ and Fortran: 
# Eigen, a C++ library, requires some code to communicate with the Fortran implementation of BLAS and LAPACK
# Pay attention to the E in the end.
find_package(LAPACKE REQUIRED)

# Add additional include directories for the backend:
# - eveything in our directory 'inc'
# - lib/ directory itself for sake of completeness (if we decide to place a header next to the CMakeLists.txt)
# - modules we need (mainly the optional requirements for eigen such as blas or lapack)
include_directories(${CMAKE_CURRENT_LIST_DIR}
                    ${CMAKE_CURRENT_LIST_DIR}/inc
                    ${LAPACKE_INCLUDE_DIRS_DEP} # = include directories of lapacke and dependencies 
                    )

# Tell cmake where additional link directories are located
# We do not need this here, because the Find??? cmake files for LAPACK etc. already do this
#link_directories(${LAPACKE_LIBRARY_DIRS_DEP}) # link directories of lapacke + dependencies