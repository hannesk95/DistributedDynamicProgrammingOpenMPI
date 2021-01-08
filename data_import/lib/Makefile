all:
	@echo "This is a dummy to prevent running make without explicit target!"

# Is created when opening the CMake Project with visual studio, blows up the size 
# of the directories for sharing the code
clean: remove_build_directories
	rm -rf .vs/

# build and install appears if the inner CMakeLists.txt is used directly
remove_build_directories:
	rm -rf build/ install/

# Get rid of everything that might be left for whatever reason, then compile from scratch
compile: remove_build_directories
	mkdir -p build/
	cd build/ && cmake -DCMAKE_BUILD_TYPE=Release ..
	$(MAKE) -C build/
	$(MAKE) -C build/ install
