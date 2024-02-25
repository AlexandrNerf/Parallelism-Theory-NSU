#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#define N 2000
#define MAX_ITER 3000
#define EPSILON 1e-6

double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void simple_iteration_method(double* a, double* b, double* x) {
    
    int i, j, k;
 	double sum;
    double err;

    double* x_new = new double[8*N];
	double error = 0.0;
    for (k = 0; k < MAX_ITER; k++) {
	error = 0.0;
	#pragma omp parallel for private(i, j, sum, err) shared(a, b, x, x_new, error) num_threads(40)
        for (i = 0; i < N; i++) {
            sum = 0.0;
            for (j = 0; j < N; j++) {
                    sum += a[i*N+j] * x[j];
            }
            err = ((sum - b[i]) * (sum - b[i])) / (b[i] * b[i]);
        x[i] = (x[i] - 0.01*(sum-b[i]));
	    error += err;
        }
        if (error < EPSILON*EPSILON) {
            break;
        }
	
       
    }
    printf("%lf\n%lf\n", x[0], x[1]);
}

int main() {
    double* A = new double[N * N];
    double* B = new double[N];  
    double* x = new double[N];

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++) {
            if (i == j)
                A[i * N + j] = 2.0;
            else
                A[i * N + j] = 1.0;
        }
    }

    for (int j = 0; j < N; j++) {
        B[j] = N + 1;
        x[j] = 0.0;
    }
    double tm = cpuSecond();
    simple_iteration_method(A, B, x);
    tm = cpuSecond() - tm;
    printf("%0.6f\n", tm);
    // Вывод результата

    return 0;
}






