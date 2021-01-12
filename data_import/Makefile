TARNAME = gruppe1_send
HOST = hpc08

all:
	@echo "This is a dummy to prevent running make without explicit target!"

clean: remove_build_directories remove_vs_folder remove_clion_build
	$(MAKE) -C lib/ clean

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

# makes sure that "parameters.pickle" is converted to "parameters.npy"
convert_pickle:
	cd ../data/data_debug/ && python3 convert_pickle.py

pack: clean
	rm -f ../$(TARNAME).tar.gz
	cd .. && tar -czf $(TARNAME).tar.gz data_import/ data/

unpack:
	mkdir $(TARNAME)
	tar -zxvf $(TARNAME).tar.gz -C $(TARNAME)
	rm Makefile $(TARNAME).tar.gz

send: pack
	scp ../$(TARNAME).tar.gz $(HOST):~/lrz-nashome/
	

# to extract Makefile to be able to unpack files
# tar -zxvf ($TARNAME).tar.gz data_import/Makefile --strip-components 1