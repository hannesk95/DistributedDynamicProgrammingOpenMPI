# `data_import`

contains with <a href="main.cpp" target="_blank">`main.cpp`</a> an example how to imread `.npy`-files directly into C++

## The scripts

This README gives a brief overview about all provided scripts and their respective tasks. For even more detailed description on the code, please refer to the respective script itself

<br/><br/>

-------

## <a href='lib/' target='_blank'>`lib/`</a>

folder containing all libraries that are used by <a href="main.cpp" target="_blank">`main.cpp`</a>

<br/><br/>

-------

## <a href='CMakeLists.txt' target='_blank'>`CMakeLists.txt`</a>

respective `cmake` file that gets called during compilation process in <a href='Makefile' target='_blank'>`Makefile`</a>

Given functionality:
- sets all parameters for the `compiler` to run

<br/><br/>

-------

## <a href='main.cpp' target='_blank'>`main.cpp`</a>

Testing script, to show check how to imread `.npy`-files directly into C++. Will later be expanded with the implemented `Async VI`

Given functionality:
- hard coded initialization of `values`, `row_pointer`, `rows`, `rshape`, `PI_star`
- import of all those parameters via <a href="https://github.com/rogersce/cnpy" target="_blank">`cnpy by Carl Rogers`</a>
- compares respective parameters if import worked correctly
- has `Alex'` version of asynchronous value iteration implemented (acutal implementation in <a href="lib/inc/Async_VI.h" target="_blank">`inc/`</a> and <a href="lib/src/Async_VI.cpp" target="_blank">`src/`</a>) to check whether everything else works. Has to be changed!

<br/><br/>

-------

## <a href='Makefile' target='_blank'>`Makefile`</a>

Makefile for this folder. Needs no change for other tasks

Given functionality:
- `make clean` - cleans project folder by removing old executables, `tar`-archives and further folders, calls `remove_pycache` and `remove_srcfiles`. Calls `make clean` in <a href='lib/' target='_blank'>`lib/`</a>
- `remove_clion_build` - removes the clion build folders [`cmake-build-debug/` and `.idea/`]
- `remove_build_directories` - removes all build folders and files [`build/`, `debug/`, `release/` and `CMakeLists.txt.user`]
- `remove_vs_folder` - removes Visual Studio folder [`.vs/`]
- `make compile` - deletes old build directories and recompiles full project. Builds `build/` folder, runs `cmake`, `make` and `make install` in there
- `convert_pickle` - makes sure that <a href='../data/data_debug/parameters.pickle' target='_blank'>`parameters.pickle`</a> is converted to <a href='../data/data_debug/parameters.npy' target='_blank'>`parameters.npy`</a>
- `pack` - packs <a href="../data/" target="_blank">`data/`</a> and <a href="../data_import/" target="_blank">`data_import/`</a> into `gruppe1.tar.gz` (in <a href="../" target="_blank">`parent folder`</a>) after deleting the old `gruppe1.tar.gz`
- `unpack` - unpacks `grupp1.tar.gz` on Eikon Computer (or anywhere else)
- `send` - sends `gruppe1.tar.gz` onto selected host (in <a href="Makefile" target="_blank">`Makefile`</a>, currently `hpc08`)