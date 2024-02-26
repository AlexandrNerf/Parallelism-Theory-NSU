#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <omp.h>

#define N 2250
#define MAX_ITER 3000
#define EPSILON 1e-6

using namespace std;

double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void simple_iteration_method_1(double* a, double* b, double* x, int thr)
{
    double sum = 0.0, error = 0.0, err = 0.0, err_dnm = 0.0;
    int i, j, k;
    for (k = 0; k < MAX_ITER; k++)
	{
		error = 0.0;
		#pragma omp parallel for private(i, j, sum, err, err_dnm) shared(a, b, x, error) num_threads(thr)
        for (i = 0; i < N; i++)
		{
            sum = 0.0;
            for (j = 0; j < N; j++)
			{
                sum += a[i*N+j] * x[j];
            }
            err += (sum - b[i]) * (sum - b[i]);
            err_dnm += b[i] * b[i];
			x[i] = (x[i] - 0.01*(sum-b[i]));
        }

        error = err / err_dnm;

        if (error < EPSILON*EPSILON)
		{
            break;
        }   
    }
}

void simple_iteration_method_2(double* a, double* b, double* x, int thr)
{
    double sum = 0.0, error = 0.0, err = 0.0, err_dnm = 0.0;

    int i, j, k;
    #pragma omp parallel shared(a, b, x, error) private(i, j, sum, err, err_dnm) num_threads(thr)
    {
        for (k = 0; k < MAX_ITER; k++)
        {
            error = 0.0;
            for (i = 0; i < N; i++)
            {
                sum = 0.0;
                for (j = 0; j < N; j++)
                {
                    sum += a[i*N+j] * x[j];
                }
                err += (sum - b[i]) * (sum - b[i]);
                err_dnm += b[i] * b[i];
                x[i] = (x[i] - 0.01*(sum-b[i]));
            }

            error = err / err_dnm;

            if (error < EPSILON*EPSILON)
            {
                break;
            }   
        }
    }
}


int main()
{
    ofstream file("result.txt");
    
    double* A = new double[N * N];
    double* B = new double[N];    
    double* x = new double[N];    

    int num_procs = omp_get_num_procs(); // Получаем количество доступных ядер
    int max_threads = num_procs > 1 ? num_procs : 1;

    for (i = 0; i < N; i++)
    {
	for (j = 0; j < N; j++)
		{
	    if (i == j)
		A[i * N + j] = 2.0;
	    else
		A[i * N + j] = 1.0;
	}
    }
    for (int th = 1; th <= 2; w++)
    {
        file << th << endl;
        for (int THREAD = 1; THREAD <= max_threads; thr++)
        {
            int i, j;
            for (j = 0; j < N; j++)
	    {
	        B[j] = N+1;
	        x[j] = 0.0;
            } 
        
            double tm = cpuSecond();
            if (th == 1)
            {
                simple_iteration_method_1(A, B, x, THREAD);
            }
            else
            {
                simple_iteration_method_2(A, B, x, THREAD);
            }
            tm = cpuSecond() - tm;
            file << fixed << setprecision(6) << tm << endl;
        }
    }

    file.close();
    return 0;
}






