TARNAME = gruppe1_send
HOST = hpc08

all:
	@echo "This is a dummy to prevent running make without explicit target!"

clean: remove_build_directories remove_vs_folder remove_clion_build

remove_clion_build:
	rm -rf cmake-build-debug
	rm -rf .idea

remove_build_directories:
	rm -rf build debug release
	rm -f CMakeLists.txt.user

remove_vs_folder:
	rm -rf .vs/

compile: remove_build_directories convert_pickle
	mkdir -p build/
	cd build/ && cmake -DCMAKE_BUILD_TYPE=Release ..
	$(MAKE) -C build/

run_all: run_debug run_small run_normal

run_debug: compile
	mkdir -p results/data_debug
	cd build/ && mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe "../data/data_debug" "../results/data_debug" 10
	cd results/ && python3 benchmark_visual.py "data_debug/"

run_small: compile
	mkdir -p results/data_small
	cd build/ && mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe "../data/data_small" "../results/data_small" 3
	cd results/ && python3 benchmark_visual.py "data_small"

run_normal: compile
	mkdir -p results/data_normal
	cd build/ && mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe "../data/data_normal" "../results/data_normal" 1	
	cd results/ && python3 benchmark_visual.py "data_normal"

run_debug_local: compile
	mkdir -p results/data_debug
	cd build/ && mpirun -np 2 ./MPI_Project.exe "../data/data_debug" "../results/data_debug" 10
	cd results/ && python3 benchmark_visual.py "data_debug/"

convert_pickle:
	cd data/ && python3 convert_pickle.py
