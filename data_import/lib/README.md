# `lib`

folder containing all libraries that are used by <a href="../main.cpp" target="_blank">`main.cpp`</a>

<br/><br/>

-------

## <a href='cmake/' target='_blank'>`cmake/`</a>

contains all `cmake` files to find used libraries (`C++` libraries like `LAPACKE` or `MKL`). Maybe needs a change for other tasks, depending on the needed `C++` libraries

<br/><br/>

-------

## <a href='inc/' target='_blank'>`inc/`</a>

contains all `include` files (Header) for project

<br/><br/>

-------

## <a href='src/' target='_blank'>`src/`</a>

contains all `src` files for project

<br/><br/>

-------

## <a href='CMakeLists.txt' target='_blank'>`CMakeLists.txt`</a>

respective `cmake` file that gets called during compilation process in <a href='Makefile' target='_blank'>`Makefile`</a>

Given functionality:
- sets all parameters for the `compiler` to run

<br/><br/>

-------

## <a href='Makefile' target='_blank'>`Makefile`</a>

Makefile for this folder

Given functionality:
- `make clean` - cleans project folder by removing build folders and Visual Studio folder [`.vs/`]
- `remove_build_directories` - removes all build folders [`build/` and `install/`]
- `make compile` - deletes old build directories and recompiles full project. Builds `build/` folder, runs `cmake`, `make` and `make install` in there