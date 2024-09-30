#include <vector>
#include <cublas_v2.h>
#include <cuda_runtime.h>


class Laplace {
private:
    double* A, * Anew;
    int m, n;

public:
    Laplace(int m, int n);
    ~Laplace();
    void init();
    double calcNext(cublasHandle_t handle);
    void swap();
};