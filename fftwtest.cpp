#include <fftw3.h>
#include <iostream>
#include <iomanip>
#include <cstddef>
#include <math.h>
#define N 8

int main()
{
    int i,j;
    fftw_complex in[N], out[N];
    fftw_plan p;

    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

        for (i = 0; i < N; i++) 
        {
        in[i][0] = cos(3 * 2*M_PI*i/N);
        in[i][1] = 0;
        }

    fftw_execute(p);

    for (j = 0; j < N; j++)
    {
    std::cout << std::setprecision(3) << out[j][0] << ", " << out[j][1] << std::endl;   
    }
 
    fftw_destroy_plan(p);

    return 0;
}