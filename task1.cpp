#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>

double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void matrix_vector_product(double* a, double* b, double* c, size_t m, size_t n)
{
    for (int i = 0; i < m; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
}

void matrix_vector_product_thread(double* a, double* b, double* c, size_t m, size_t n, int threadid, int nthreads)
{
    int items_per_thread = m / nthreads;
    int lb = threadid * items_per_thread;
    int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
    for (int i = lb; i <= ub; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
}

void run_serial(size_t n, size_t m)
{
    double* a, * b, * c;
    a = new double[m * n];
    b = new double[n];
    c = new double[m];

    if (a == NULL || b == NULL || c == NULL)
    {
        free(a);
        free(b);
        free(c);
        std::cout << error;
        exit(1);
    }
    double t = cpuSecond();
    for (size_t i = 0; i < m; i++)
    {
        for (size_t j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }

    for (size_t j = 0; j < n; j++)
        b[j] = j;

    matrix_vector_product(a, b, c, m, n);
    t = cpuSecond() - t;
}

void run_parallel(size_t n, size_t m, int thr)
{
    double* a, * b, * c;

    a = new double[m * n];
    b = new double[n];
    c = new double[m];

    if (a == NULL || b == NULL || c == NULL)
    {
        exit(1);
    }
    double t = cpuSecond();

    int nthreads = thr;

    std::thread* threads = new std::thread[nthreads];
    for (int i = 0; i < nthreads; i++)
    {
        threads[i] = std::thread(matrix_vector_product_thread, a, b, c, m, n, i, nthreads);
    }
    for (int i = 0; i < nthreads; i++)
    {
        threads[i].join();
    }
    delete[] threads;
    delete[] a, b, c;
    t = cpuSecond() - t;

    std::cout << thr << ": " << t;
}

int main(int argc, char* argv[])
{
    size_t M = 1000;
    size_t N = 1000;
    if (argc > 1)
        M = atoi(argv[1]);
    if (argc > 2)
        N = atoi(argv[2]);
    int threadss = { 1, 2, 4, 7, 8, 16, 20, 40 };
    for (auto thr : threadss)
        run_parallel(M, N, thr);
    return 0;
}