#include "laplace2d.hpp"
#include <cmath>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cublas_v2.h>
#include <cuda_runtime.h>

#define OFFSET(x, y, m) (((x) * (m)) + (y))

Laplace::Laplace(int m, int n) : m(m), n(n)
{
    A = new double[n * m];
    Anew = new double[n * m];
}

Laplace::~Laplace()
{
#pragma acc exit data delete (this)
    delete[] A;
    delete[] Anew;
}

void Laplace::init()
{
	std::vector<std::pair<int, double>> hp({std::make_pair(n + 1, 10),
													std::make_pair(2 * n - 2 , 20),
													std::make_pair(n * n - 2 * n + 1, 30),
													std::make_pair(n * n - n - 2, 40)
													});
    memset(A, 0, n * m * sizeof(double));
    memset(Anew, 0, n * m * sizeof(double));

    for (auto h : hp) {
        int index = h.first;
        double temp = h.second;
        A[index] = temp;
        Anew[index] = temp;
    }

    for (int i = 2; i < n - 2; i++) {
        A[OFFSET(1, i, n)] = hp[0].second + (hp[1].second - hp[0].second) * i / (n - 1);
        A[OFFSET(m-2, i, n)] = hp[3].second + (hp[2].second - hp[3].second) * i / (n - 1);
        A[OFFSET(i, 1, n)] = hp[0].second + (hp[3].second - hp[0].second) * i / (n - 1);
        A[OFFSET(i, n-2, n)] = hp[1].second + (hp[2].second - hp[1].second) * i / (n - 1);
    }
#pragma acc enter data copyin(this)
#pragma acc enter data copyin(A[ : n * m], Anew[ : n * m])
}

double Laplace::calcNext(cublasHandle_t handle)
{
    double error = 0.0;
    double* d_errors;

    // Allocate memory for errors on the device
    cudaMalloc((void**)&d_errors, (n-2) * sizeof(double));

#pragma acc enter data copyin(A[ : n * m], Anew[ : n * m])

#pragma acc parallel loop present(A, Anew) deviceptr(d_errors)
    for (int j = 1; j < n - 1; j++) {
#pragma acc loop
        for (int i = 1; i < m - 1; i++) {
            int points = 5;
            if (i == 1 || i == m - 2) points--;
            if (j == 1 || j == n - 2) points--;
            Anew[OFFSET(j, i, n)] = (A[OFFSET(j, i + 1, n)] + A[OFFSET(j, i - 1, n)] +
                                     A[OFFSET(j - 1, i, n)] + A[OFFSET(j + 1, i, n)] +
                                     A[OFFSET(j, i, n)]) / points;
            d_errors[j - 1]  = fmax(error, fabs(Anew[OFFSET(j, i, n)] - A[OFFSET(j, i, n)]));
        }


    }
#pragma acc wait
#pragma acc exit data delete(A, Anew)
    // Use cuBLAS to find the maximum error


    int maxIndex;
#pragma acc data copyin(d_errors[ : (n-2)])
    {
        cublasIdamax(handle, (n-2), d_errors, 1, &maxIndex);
    }

    cudaMemcpy(&error, &d_errors[maxIndex - 1], sizeof(double), cudaMemcpyDeviceToHost);
cudaFree(d_errors);
//    cublasDestroy(handle);

    return error;
}

void Laplace::swap()
{
#pragma acc parallel loop present(A, Anew)
    for (int j = 1; j < n - 1; j++) {
#pragma acc loop
        for (int i = 1; i < m - 1; i++) {
            A[OFFSET(j, i, m)] = Anew[OFFSET(j, i, m)];
        }
    }
}
