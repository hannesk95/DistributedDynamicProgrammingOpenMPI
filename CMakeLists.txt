cmake_minimum_required (VERSION 3.0)
project(MPI_Project.exe)

# Prevent compilation in-source
if( ${CMAKE_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR} )
  Message( "" )
  Message( FATAL_ERROR "Source and build directories are the same!")
endif()

# Flags for release configuration: 
# -O3:           level 3 compiler optimizations => faster code
# -march-native: compile for the native (=your computer) architecture => faster code
# -ffast-math:   allow "unsafe" match operations, i.e. those operations, where the result of a floating point operation could be unstable
#                This might be a problem for cross platform but deterministic applications (e.g. multiplayer games)
#                allow fast math => faster code
# -fopenmp:      activate openmp support (next lecture), might be enabled by default depending on your compiler
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math -fopenmp")


# I tried out PicklingTools but it is crap
#include(ExternalProject)
#ExternalProject_Add(
#        libptools
#        DEPENDS ""
#        SOURCE_DIR ${PROJECT_SOURCE_DIR}/PicklingTools170Release/C++
#        CONFIGURE_COMMAND ""
#        BUILD_COMMAND make libptools.so
#        BUILD_IN_SOURCE 1
#        INSTALL_COMMAND ""
#        )
#set(PICKLINGTOOLS_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/PicklingTools170Release/C++ ${PROJECT_SOURCE_DIR}/PicklingTools170Release/C++/opencontainers_1_8_5/include)
#file(GLOB PICKLINGTOOLS_LIBRARIES ${PROJECT_SOURCE_DIR}/PicklingTools170Release/C++/*.so)


# If we wanted to use boost with python the following would be needed:
#find_package(PythonLibs REQUIRED)
# For some external project macros
#include(ExternalProject)
#set( boost_URL "http://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.bz2" )
#set( boost_SHA1 "9f1dd4fa364a3e3156a77dc17aa562ef06404ff6" )
# Download boost from git and build regex module
#ExternalProject_Add(
#    boost
#    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/boost
#    URL ${boost_URL}
#    URL_HASH SHA1=${boost_SHA1}
#    CONFIGURE_COMMAND ./bootstrap.sh
#            --with-libraries=all
#            --prefix=<INSTALL_DIR>
#    BUILD_COMMAND ./b2 install link=static variant=release threading=multi runtime-link=static install 
#    BUILD_IN_SOURCE 1
#    INSTALL_COMMAND ""
#    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/boostinstall
#)
#set(BOOST_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/boostinstall/include)
#file(GLOB BOOST_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/boostinstall/lib/*.a)







# Additional 'find module' scipts in the subfolder 'cmake'
# Currently, FindLAPACKE is the important one. FindMKL exist for the case that you need it in another project.
#set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/cmake") #/;${CMAKE_MODULE_PATH}")
#message("${CMAKE_CURRENT_LIST_DIR}/lib/cmake/") #;${CMAKE_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/cmake")


# We want parallel code
find_package(OpenMP REQUIRED)

# Open MPI Package
find_package(MPI REQUIRED)

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

# CNPY needs zlib
find_package(ZLIB REQUIRED)

# Path to cnpy directory
set(CNPY_DIR lib/cnpy)

add_subdirectory(${CNPY_DIR})

# Add Include directories for cnpy
include_directories(${CNPY_DIR} 
                    ${MPI_INCLUDE_PATH} 
                    ${ZLIB_INCLUDE_DIRS} 
                    ${LAPACKE_INCLUDE_DIRS_DEP} 
                    ${CMAKE_CURRENT_LIST_DIR}/lib/Eigen/
		    ${CMAKE_CURRENT_LIST_DIR}/lib/
		    ${CMAKE_CURRENT_LIST_DIR}/src/
		    #${BOOST_INCLUDE_DIRS}
                    #${PYTHON_INCLUDE_DIRS}
                    #${PICKLINGTOOLS_INCLUDE_DIRS}
                    )

# link_directories(${CMAKE_CURRENT_LIST_DIR}/lib/Eigen)

# Create executable as usual
file(GLOB SRC_FILES *.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

#add_dependencies(${PROJECT_NAME} boost)
target_link_libraries(${PROJECT_NAME} 
                      ${MPI_LIBRARIES} 
                      ${ZLIB_LIBRARIES} 
                      OpenMP::OpenMP_CXX 
                      ${LAPACKE_LIBRARIES_DEP} 
                      #${BOOST_LIBRARIES} 
                      #${PYTHON_LIBRARIES}
                      #${PICKLINGTOOLS_LIBRARIES}
                      cnpy
                      )
