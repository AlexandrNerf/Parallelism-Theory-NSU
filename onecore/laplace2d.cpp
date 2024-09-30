
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "laplace2d.hpp"
#include <vector>

#define OFFSET(x, y, m) (((x) * (m)) + (y))

Laplace::Laplace(int m, int n) : m(m), n(n)
{
    A = new double[n * m];
    Anew = new double[n * m];
}

Laplace::~Laplace()
{
    delete (A);
    delete (Anew);
}

void Laplace::init()
{
	std::vector<std::pair<int, double>> hp({std::make_pair(n + 1, 10),
                                                      std::make_pair(2 * n - 2, 20),
                                                      std::make_pair(n * n - 2 * n + 1, 30),
                                                      std::make_pair(n * n - n - 2, 40)
                                                     });
    memset(A, 0, (n * n) * sizeof(double));
    memset(Anew, 0, n * n * sizeof(double));
    for (auto h : hp) {
        int index = h.first;
        double temp = h.second;
        A[index] = temp;
        Anew[index] = temp;
    }
    for (int i=2;i<n-2;i++) {

        A[OFFSET(1,i,n)] = hp[0].second + (hp[1].second - hp[0].second) * i / (n - 1);

        A[OFFSET(-2,i,n)] = hp[3].second + (hp[2].second - hp[3].second) * i / (n - 1);

        A[OFFSET(i,1,n)] = hp[0].second + (hp[3].second - hp[0].second) * i / (n - 1);

        A[OFFSET(i,-2,n)] = hp[1].second + (hp[2].second - hp[1].second) * i / (n - 1);
    }
}

double Laplace::calcNext()
{
    double error = 0.0;
    for (int j = 1; j < n - 1; j++) {
        for (int i = 1; i < n - 1; i++) {
            int points = 1;
            points += i > 1 ? 1 : 0;
            points += j > 1 ? 1 : 0;
            points += i < n - 2 ? 1 : 0;
            points += j < n - 2 ? 1 : 0;
            Anew[OFFSET(j, i, n)] = (A[OFFSET(j, i + 1, n)] + A[OFFSET(j, i - 1, n)] +
                                     A[OFFSET(j - 1, i, n)] + A[OFFSET(j + 1, i, n)] +
                                     A[OFFSET(j, i, n)])  / points;
            error = fmax(error, fabs(Anew[OFFSET(j, i, n)] - A[OFFSET(j, i, n)]));

        }
    }
    return error;
}

void Laplace::swap()
{
    for( int j = 1; j < n-1; j++)
    {
        for( int i = 1; i < m-1; i++ )
        {
            A[OFFSET(j, i, m)] = Anew[OFFSET(j, i, m)];
        }
    }

}