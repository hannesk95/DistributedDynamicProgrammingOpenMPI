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

compile: remove_build_directories convert_pickle documentation
	mkdir -p build/
	cd build/ && cmake -DCMAKE_BUILD_TYPE=Release ..
	$(MAKE) -C build/

run_all: run_debug run_small run_normal

run_debug: compile
	for NP in 2 4 6 8 ; do \
		mkdir -p results/data_debug-$$NP ; \
		cd build/ && mpirun -np $$NP -hostfile ../hostfile ./MPI_Project.exe "../data/data_debug" "../results/data_debug-$$NP" 10 ; \
		cd .. ; \
		cd results/ && python3 benchmark_comm_period.py "data_debug-$$NP/" ; \
		cd .. ; \
	done
	python3 results/benchmark_np.py "results/"

run_small: compile
	for NP in 2 4 6 8 ; do \
		mkdir -p results/data_small-$$NP ; \
		cd build/ && mpirun -np $$NP -hostfile ../hostfile ./MPI_Project.exe "../data/data_small" "../results/data_small-$$NP" 3 ; \
		cd .. ; \
		cd results/ && python3 benchmark_comm_period.py "data_small-$$NP/" ; \
		cd .. ; \
	done
	python3 results/benchmark_np.py "results/"

run_normal: compile
	for NP in 2 4 6 8 ; do \
		mkdir -p results/data_normal-$$NP ; \
		cd build/ && mpirun -np $$NP -hostfile ../hostfile ./MPI_Project.exe "../data/data_normal" "../results/data_normal-$$NP" 1	 ; \
		cd .. ; \
		cd results/ && python3 benchmark_comm_period.py "data_normal-$$NP/" ; \
		cd .. ; \
	done
	python3 results/benchmark_np.py "results/"

run_debug_local: compile
	for NP in 2 4 ; do \
		mkdir -p results/data_debug-$$NP ; \
		cd build/ && mpirun -np $$NP ./MPI_Project.exe "../data/data_debug" "../results/data_debug-$$NP" 10 ; \
		cd .. ; \
		cd results/ && python3 benchmark_comm_period.py "data_debug-$$NP/" ; \
		cd .. ; \
	done
	python3 results/benchmark_np.py "results/"

convert_pickle:
	cd data/ && python3 convert_pickle.py

documentation:
	doxygen Doxyfile
