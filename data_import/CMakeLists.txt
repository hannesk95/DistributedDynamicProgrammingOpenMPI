# Always specify which version you need:
# - at least VERSION 3
# - EIKON has VERSION 3.10.2 available
cmake_minimum_required (VERSION 3.10)

# Compiler flags for all configuration, -Wall enables all warnings
set(CMAKE_CXX_FLAGS "-Wall -L${CMAKE_CURRENT_LIST_DIR}/lib/inc/cnpy")

# Flags for release configuration: 
# -O3:           level 3 compiler optimizations => faster code
# -march-native: compile for the native (=your computer) architecture => faster code
# -ffast-math:   allow "unsafe" match operations, i.e. those operations, where the result of a floating point operation could be unstable
#                This might be a problem for cross platform but deterministic applications (e.g. multiplayer games)
#                allow fast math => faster code
# -fopenmp:      activate openmp support (next lecture), might be enabled by default depending on your compiler
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math -fopenmp")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_INSTALL_RPATH ${CMAKE_CURRENT_LIST_DIR}/lib/inc/cnpy/)
#set(DEST_INC ${CMAKE_CURRENT_LIST_DIR}/lib/inc/cnpy/)

# The name for this library, let's be super creative and call it 'backend'
set(LIB_NAME backend)

# The name for the c++ test program (mainly for debugging with an IDE, the debugger needs an entry point to the library)
project(data_import)

# Prevent compilation in-source directory, if compiled in console
if( ${CMAKE_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR} )
    Message( FATAL_ERROR "Source and build directories are the same!")
endif()

# To make use of the our library add a subdirectory call to build and install it
# -> no need to care about the library when using an IDE to "open" this CMakeLists.txt
add_subdirectory(lib)

# Avoid annoying paths in main.cpp by providing the additional include directories
include_directories(lib/inc)

# We need the MPI Package, it is installed in a way that you can find it via CMake
find_package(MPI REQUIRED)

# Add the includes for MPI
include_directories(${MPI_INCLUDE_PATH})

# All additional source files for debugging, e.g. if the debugging code is located in different files
set(SRC_FILES main.cpp)

# Create an executable as usual, especially debugging C++ code with Qt becomes easy
add_executable(${PROJECT_NAME} ${SRC_FILES})

# linking cnpy and z library
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,${CMAKE_CURRENT_LIST_DIR}/lib/inc/cnpy")
target_link_libraries(${PROJECT_NAME} cnpy z ${LIB_NAME} ${MPI_LIBRARIES} -lpthread)

