TARNAME = gruppe1_send
HOST = hpc08

all:
	@echo "This is a dummy to prevent running make without explicit target!"

clean: remove_build_directories remove_vs_folder remove_clion_build

# The folders and files that belong to CLion only
remove_clion_build:
	rm -rf cmake-build-debug
	rm -rf .idea

# The folders and files that qtcreator creates (assuming you create those in here)
remove_build_directories:
	rm -rf build debug release
	rm -f CMakeLists.txt.user
	
# Is created when opening the CMake Project with visual studio, blows up the size 
# of the directories for sharing the code due to VS database
remove_vs_folder:
	rm -rf .vs/

# Get rid of everything that might be left for whatever reason, then compile from scratch
compile: remove_build_directories convert_pickle
	mkdir -p build/
	cd build/ && cmake -DCMAKE_BUILD_TYPE=Release ..
	$(MAKE) -C build/

run: compile run_debug run_small run_normal

run_debug: compile
	cd build/ && mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe "../data/data_debug" "../results/data_debug" 20
	python3 benchmark_visual.py "/results/data_debug/"

run_small: compile
	cd build/ && mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe "../data/data_small" "../results/data_small" 5
	python3 benchmark_visual.py "/results/data_small"

run_normal: compile
	cd build/ && mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe "../data/data_normal" "../results/data_normal" 1	
	python3 benchmark_visual.py "/results/data_normal"

run_mpi_local: compile
	cd build/ && mpirun -np 2 ./MPI_Project.exe

run_mpi_distr: compile
	cd build/ && mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe "../data/data_debug" "../results/data_debug"

# visualize results
visual_benchmark:
	python3 benchmark_visual.py

# makes sure that "parameters.pickle" is converted to "parameters.npy"
convert_pickle:
	cd data/ && python3 convert_pickle.py
