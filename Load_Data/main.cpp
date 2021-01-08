#include <iostream>
#include "cnpy.h"
#include <complex>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cstring>

int main() {
    std::cout << "Hello, World!" << std::endl;

    FILE *cp = fopen("test.txt", "rb");

    std::string numpy_file = "array.npy";

    FILE *cp1 = fopen(numpy_file.c_str(), "rb");

    cnpy::NpyArray arr = cnpy::npy_load("./array.npy");
    std::complex<double>* loaded_data = arr.data<std::complex<double>>();


    return 0;
}
