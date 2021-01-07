// A simple program that checks whether the C++ implementation is working with "data_debug/" dataset

// Imports
#include <iostream> // for print statements
#include <vector> // for vector operations
#include <numeric> // to calculate the difference of two vectors
#include "cnpy/cnpy.h"
#include "Async_VI.h" // for Asnyc value iteration

int main(int argc, char *argv[])
{
    // original values of probability matrix
    std::vector<double> values({1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. , 1. ,
                                0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. ,
                                1. , 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. ,
                                0.5, 0.5, 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. , 0.5, 0.5,
                                1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. , 0.5, 0.5, 1. , 1. ,
                                1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 0.5,
                                0.5, 1. , 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5,
                                0.5, 0.5, 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 1. , 0.5,
                                0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 1. , 0.5,
                                0.5, 1. , 1. , 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 1. , 1. , 0.5, 0.5,
                                1. , 0.5, 0.5, 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 1. , 1. , 1. , 0.5, 0.5, 1. , 0.5, 0.5, 0.5, 0.5, 1. , 1. , 0.5, 0.5, 1. , 1. , 1. , 1.});
    // original index pointer indices of probability matrix
    std::vector<int> row_pointer({0,   1,   1,   1,   1,   2,   2,   2,   2,   3,   3,   3,   3, 4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,
                           7,   7,   7,   8,   8,   8,   8,   9,   9,   9,   9,  10,  10, 10,  10,  11,  11,  11,  11,  12,  12,  12,  12,  13,  13,  13,
                           13,  14,  14,  14,  14,  15,  15,  15,  15,  16,  16,  16,  16, 17,  17,  17,  17,  18,  18,  18,  18,  19,  19,  19,  19,  20,
                           20,  20,  20,  21,  21,  21,  21,  22,  22,  22,  22,  23,  23, 23,  23,  24,  24,  24,  24,  25,  25,  25,  25,  26,  28,  28,
                           28,  29,  29,  29,  29,  30,  32,  32,  32,  33,  33,  33,  33, 34,  34,  34,  34,  35,  37,  37,  37,  38,  38,  38,  38,  39,
                           41,  41,  41,  42,  42,  42,  42,  43,  43,  43,  43,  44,  46, 46,  46,  47,  47,  47,  47,  48,  50,  50,  50,  51,  51,  51,
                           51,  52,  52,  52,  52,  53,  55,  55,  55,  56,  56,  56,  56, 57,  59,  59,  59,  60,  60,  60,  60,  61,  61,  61,  61,  62,
                           64,  64,  64,  65,  65,  65,  65,  66,  68,  68,  68,  69,  69, 69,  69,  70,  70,  70,  70,  71,  72,  74,  74,  75,  77,  77,
                           77,  78,  80,  80,  80,  81,  81,  81,  81,  82,  82,  82,  82, 83,  84,  86,  86,  87,  89,  89,  89,  90,  92,  92,  92,  93,
                           93,  93,  93,  94,  94,  94,  94,  95,  96,  98,  98,  99, 101, 101, 101, 102, 104, 104, 104, 105, 105, 105, 105, 106, 106, 106,
                           106, 107, 108, 110, 110, 111, 113, 113, 113, 114, 116, 116, 116, 117, 117, 117, 117, 118, 118, 118, 118, 119, 120, 122, 122, 123,
                           125, 125, 125, 126, 128, 128, 128, 129, 129, 129, 129, 130, 130, 130, 130, 131, 132, 134, 134, 135, 137, 139, 139, 140, 142, 143,
                           143, 144, 144, 144, 144, 145, 145, 145, 145, 146, 147, 149, 149, 150, 152, 154, 154, 155, 157, 158, 158, 159, 159, 159, 159, 160,
                           160, 160, 160, 161, 162, 164, 164, 165, 167, 169, 169, 170, 172, 173, 173, 174, 174, 174, 174, 175, 175, 175, 175, 176, 177, 179,
                           179, 180, 182, 184, 184, 185, 187, 188, 188, 189, 189, 189, 189, 190, 190, 190, 190, 191, 192, 194, 194, 195, 197, 199, 199, 200,
                           202, 203, 203, 204, 204, 204, 204, 205, 205, 205, 205, 206, 207, 209, 209, 210, 212, 214, 215, 216, 218, 219, 219, 220, 220, 220,
                           220, 221, 222, 222, 222, 223, 224, 226, 226, 227, 229, 231, 232, 233, 235, 236, 236, 237, 237, 237, 237, 238, 239, 239, 239, 240,
                           241, 243, 243, 244, 246, 248, 249, 250, 252, 253, 253, 254, 254, 254, 254, 255, 256, 256, 256, 257, 258, 260, 260, 261, 263, 265, 266, 267, 269, 270, 270,
                           271, 271, 271, 271, 272, 273, 273, 273, 274, 275, 277, 277, 278, 280, 282, 283, 284, 286, 287, 287, 288, 288, 288, 288, 289, 290, 290, 290});
    // original row of probability matrix
    std::vector<int> rows({ 0,  26,   2,  28,   4,   5,  31,   7,  33,   9,  10,  36,  12, 38,  14,  15,  41,  17,  43,  19,  20,  46,  22,  48,  24,  25,
                                  0,   2,  51,  27,   0,   2,  53,  29,  30,   5,   7,  56,  32, 5,   7,  58,  34,  35,  10,  12,  61,  37,  10,  12,  63,  39,
                                  40,  15,  17,  66,  42,  15,  17,  68,  44,  45,  20,  22,  71, 47,  20,  22,  73,  49,  50,  76,  25,  27,  76,  25,  27,  52,
                                  25,  27,  78,  54,  55,  81,  30,  32,  81,  30,  32,  57,  30, 32,  83,  59,  60,  86,  35,  37,  86,  35,  37,  62,  35,  37,
                                  88,  64,  65,  91,  40,  42,  91,  40,  42,  67,  40,  42,  93, 69,  70,  96,  45,  47,  96,  45,  47,  72,  45,  47,  98,  74,
                                  75, 101,  50,  52, 101,  50,  52,  50,  52,  77,  50,  52, 101, 103,  79,  80, 106,  55,  57, 106,  55,  57,  55,  57,  82,  55,
                                  57, 106, 108,  84,  85, 111,  60,  62, 111,  60,  62,  60,  62, 87,  60,  62, 111, 113,  89,  90, 116,  65,  67, 116,  65,  67,
                                  65,  67,  92,  65,  67, 116, 118,  94,  95, 121,  70,  72, 121, 70,  72,  70,  72,  97,  70,  72, 121, 123,  99, 100, 101,  75,
                                  77, 101,  75,  77,  75,  77,  79, 102,  75,  77, 101, 103, 104, 101, 105, 106,  80,  82, 106,  80,  82,  80,  82,  84, 107,  80,
                                  82, 106, 108, 109, 106, 110, 111,  85,  87, 111,  85,  87,  85, 87,  89, 112,  85,  87, 111, 113, 114, 111, 115, 116,  90,  92,
                                  116,  90,  92,  90,  92,  94, 117,  90,  92, 116, 118, 119, 116, 120, 121,  95,  97, 121,  95,  97,  95,  97,  99, 122,  95,  97, 121, 123, 124, 121});
    // original shape
    std::vector<int> rshape({500,125});
    // original PI_star
    std::vector<int> PI_star({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 2, 0, 0, 2, 1, 0,
                                 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 1, 1, 0, 1, 1, 0, 2, 0, 1, 2, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 3, 2, 0, 0});

    // init SparseMatrix
    Eigen::Map<SpMat> probabilities_map(rshape[0], rshape[1], values.size(), row_pointer.data(), rows.data(), values.data());

    // inread the files
    cnpy::NpyArray raw_indptr = cnpy::npy_load("../../data/data_debug/P_indptr.npy");
    cnpy::NpyArray raw_indices = cnpy::npy_load("../../data/data_debug/P_indices.npy");
    cnpy::NpyArray raw_data = cnpy::npy_load("../../data/data_debug/P_data.npy");
    cnpy::NpyArray raw_shape = cnpy::npy_load("../../data/data_debug/P_shape.npy");
    cnpy::NpyArray raw_pistar = cnpy::npy_load("../../data/data_debug/pi_star_alpha_0_99_iter_1000.npy");

    // convert the content
    std::vector<int> indptr = raw_indptr.as_vec<int>();
    std::vector<int> indices = raw_indices.as_vec<int>();
    std::vector<float> data_float = raw_data.as_vec<float>();
    // convert float vector to double vector -> to init SparseMatrix
    std::vector<double> data(data_float.begin(), data_float.end());
    std::vector<int64_t> shape = raw_shape.as_vec<int64_t>();
    std::vector<int> pi_star = raw_pistar.as_vec<int>();

    // init SparseMatrix, check whether it works
    Eigen::Map<SpMat> probabilities2_map(shape[0], shape[1], data.size(), indptr.data(), indices.data(), data.data());

    //
    // check whether all values are parsed correctly
    //
    
    std::vector<double> ndices;
    std::vector<double> ndptr;
    std::vector<double> ata;
    std::vector<double> hape;
    std::vector<double> i_star;

    std::transform(row_pointer.begin(),row_pointer.end(),indptr.begin(),std::back_inserter(ndices),std::minus<double>());
    std::transform(rows.begin(),rows.end(),indices.begin(),std::back_inserter(ndptr),std::minus<double>());
    std::transform(values.begin(),values.end(),data.begin(),std::back_inserter(ata),std::minus<double>());
    std::transform(rshape.begin(),rshape.end(),shape.begin(),std::back_inserter(hape),std::minus<double>());
    std::transform(PI_star.begin(),PI_star.end(),pi_star.begin(),std::back_inserter(i_star),std::minus<double>());

    std::cout << "Difference between original indices and inread ones: " << std::accumulate(ndices.begin(), ndices.end(), 0) << std::endl;
    std::cout << "Difference between original index pointer and inread ones: " << std::accumulate(ndptr.begin(), ndptr.end(), 0) << std::endl;
    std::cout << "Difference between original data and inread ones: " << std::accumulate(ata.begin(), ata.end(), 0) << std::endl;
    std::cout << "Difference between original shape and inread ones: " << std::accumulate(hape.begin(), hape.end(), 0) << std::endl;
    std::cout << "Difference between original pi_star and inread ones: " << std::accumulate(i_star.begin(), i_star.end(), 0) << std::endl;

    //
    // now test the async value iteration
    //

    // init number of stars, states and actions
    const unsigned int n_stars = 5;
    const unsigned int nS = 125;
    const unsigned int nA = 4;
    // init value and policy
    std::vector<double> v;
    std::vector<double> pi;
    // fill
    for (int i=0; i < 125; i++)
    {
        v.push_back(0);
        pi.push_back(0);
    }
    // run asynchronous value iteration
    Backend::async_vi(v.data(), pi.data(), data.data(), indices.data(), indptr.data(), values.size(), shape[1], shape[0], n_stars, nS, nA);
    std::vector<double> result;
    // calc "difference" vector, print sum of "difference" vector
    std::transform(pi.begin(),pi.end(),pi_star.begin(),std::back_inserter(result),std::minus<double>());
    std::cout << "Difference between ground truth and calculated policy is " << std::accumulate(result.begin(), result.end(), 0);
    return 0;
}
