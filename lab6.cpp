#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <math.h>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <memory>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <chrono>
// PARAMETERS SECTION

// output file name
#define OUT_FILE "result.dat"

// mesh size

int n;
int iter;

// parameters
#define TAU -0.01
#define EPS 0.01

const int NUM_THREADS = 4;

// END OF PARAMETERS SECTION

using namespace std;

// get matrix value (SIZE x SIZE)
double get_a(int row, int col) {
	if (row==col) return 10;
	if (row+1==col) return 20;
	if (row-1==col) return 20;
	if (row+n==col) return 30;
	if (row-n==col) return 30;
	return 0;
}

double get_b(int idx) {
	// some heat input/output
	if (idx==n/2*n+n/3) return 10;
	if (idx==n*2/3*n+n*2/3) return -25;
	return 0;
}

void init_matrix(double *M) {
	int i, j;

	for (i=0; i<n*n; i++)
		for (j=0; j<n*n; j++)
			M[i*(n*n)+j] = get_a(i, j);
}

void init_b(double *b) {
	int i;

	for (i=0; i<n*n; i++)
		b[i] = get_b(i);
}

double norm(double *x) {
	double result=0;
	int i;

	for (i=0; i<n*n; i++)
		result += x[i]*x[i];
	return sqrt(result);
}

// res = Ax-y
void mul_mv_sub_prll(double *res, double *A, double *x, double *y) {
	std::vector<std::thread> threads;
    int chunk_size = (n*n) / NUM_THREADS;

	for (int i = 0; i < NUM_THREADS; i++) {
        int start = i * chunk_size;
        int end = (i == NUM_THREADS - 1) ? n*n : (i + 1) * chunk_size;
        threads.push_back(std::thread(mul_mv_sub, res, A, x, y, start, end));
    }

    for (auto &thread : threads) {
        thread.join();
    }
}
void mul_mv_sub(double *res, double *A, double *x, double *y, int start, int end) {

	for (int i=start; i<end; i++) {
		res[i] = -y[i];
		for (int j=0; j<n*n; j++)
			res[i] += A[i*(n*n)+j] * x[j];
	}
}



// x -= TAU * delta
void next(double *x, double *delta) {
	int i;

	for (i=0; i<(n*n); i++)
		x[i] -= TAU * delta[i];
}

void solve_simple_iter(double *A, double *x, double *b) {
	double *Axmb, norm_b, norm_Axmb;

	norm_b = norm(b);

	Axmb = new double[n*n];
	auto start = std::chrono::high_resolution_clock::now();
  	
	int iters = 0;
	do {
		mul_mv_sub_prll(Axmb, A, x, b);
		norm_Axmb=norm(Axmb);
		next(x, Axmb);
	} while (norm_Axmb/norm_b >= EPS && iters++ < iter);

  	auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> runtime = end-start;

    printf(" total: %f s\n", runtime);

	free(Axmb);
}

int main(int argc, char* argv[]) {

	po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Produce help message")
        ("precision", po::value<double>()->default_value(0.000001), "Precision")
        ("grid-size", po::value<int>()->default_value(512), "Set grid size")
        ("iterations", po::value<int>()->default_value(1000000), "Set number of iterations");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);


    double precision = vm["precision"].as<double>();
    n = vm["grid-size"].as<int>();
    iter = vm["iterations"].as<int>();

	double *A, *b, *x;

	A = new double[n*n];
	b = new double[n*n];
	x = new double[n*n];

	init_matrix(A);
	init_b(b);

	for (int i = 0; i < n*n; i++)
		x[i] = 0;

	solve_simple_iter(A, x, b);

	return 0;
}