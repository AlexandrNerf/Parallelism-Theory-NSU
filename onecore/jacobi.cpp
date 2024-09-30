
#include "laplace2d.hpp"
#include <nvtx3/nvToolsExt.h>
#include <boost/program_options.hpp>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <chrono>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    int n = 1024;
    double err = 1.0e-6;
    int iters = 1000000;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("n", po::value<int>(&n))
            ("iter", po::value<int>(&iters))
            ("err", po::value<double>(&err));

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);



    double error = 1.0;


    n+=2;
    Laplace A(n, n);
    nvtxRangePushA("init");

    A.init();
    nvtxRangePop();
    printf("Jacobi relaxation Calculation on CPU (Onecore): %d x %d mesh\n", n-2, n-2);

    auto start = std::chrono::high_resolution_clock::now();
    int iter = 0;

    nvtxRangePushA("while");
    while (error > err && iter < iters)
    {
        nvtxRangePushA("calc");
        error = A.calcNext();
        nvtxRangePop();

        nvtxRangePushA("swap");
        A.swap();
        nvtxRangePop();
        if (iter % 1000 == 0)
            printf("%5d, %0.6f\n", iter, error);

        iter++;
    }
    nvtxRangePop();

    auto tm = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
    printf("%5d, %0.6f\n", iter, error);
    std::cout << "Elapsed time: " << tm.count() / 1000000. <<std::endl;
    return 0;
}