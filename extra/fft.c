#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <complex.h>

// 3.1415926535897932
#define PI 3.141592

float pi;

/*
  A fast Fourier transform (FFT) is an algorithm that computes the discrete
  Fourier transform (DFT) of a sequence, or its inverse (IDFT). Fourier analysis
  converts a signal from its original domain (often time or space) to a
  representation in the frequency domain and vice versa. The DFT is obtained by
  decomposing a sequence of values into components of different frequencies. This
  operation is useful in many fields, but computing it directly from the
  definition is often too slow to be practical. An FFT rapidly computes such
  transformations by factorizing the DFT matrix into a product of sparse (mostly
  zero) factors. As a result, it manages to reduce the complexity of computing the
  DFT from O(n^2), which arises if one simply applies the definition of DFT, to O
  ( n log n ), where n is the data size. The difference in speed can be enormous,
  especially for long data sets where n may be in the thousands or millions. In
  the presence of round-off error, many FFT algorithms are much more accurate than
  evaluating the DFT definition directly or indirectly. There are many different
  FFT algorithms based on a wide range of published theories, from simple
  complex-number arithmetic to group theory and number theory.

  -- From Wikipedia, the free encyclopedia
*/

/*
    [ * * * * * * * * | # # # # # # # # ] => [i] == [i + n/2]
    1. For the even frequencies the second half of the results are the same of
    the first half.

    [ * * * * * * * * | # # # # # # # # ] => [i] == [i + n/2] * (-1)
    2. For the odd frequencies the second half is the same but negative.

    [ 0 * * * * * * * | 8 * * * * * * * ]
    3. For the even the freq x have the same results of the freq x + (n / 2).

    [ * 1 * * * * * * | * 9 * * * * * * ]
    4. For the odd freq x have the same results that freq x + (n / 2) but they
    are the inverse numbers (x * (-1)).

    5. in[i] * cexp(2 * I * PI * t * frequency) is the use of euler's formula
    to compute cos and sin at the same time for effieciency
 */

void fft(float in[], size_t step, float complex out[], size_t n)
{
    assert(n > 0);

    if (n == 1) {
        out[0] = in[0];
        return;
    }

    fft(in,        step * 2, out,         n / 2);
    fft(in + step, step * 2, out + n / 2, n / 2);

    for (size_t k = 0; k < n / 2; k++) {
        float t  = (float) k / n;
        float complex v = cexp(-2 * I * pi * t) * out[k + n / 2];
        float complex e = out[k];
        out[k]         = e + v;
        out[k + n / 2] = e - v;
    }
}

void dft(float in[], float complex out[], size_t n)
{
    for (size_t freq = 0; freq < n; freq++) {
        out[freq] = 0; // Accumulate the products
        for (size_t i = 0; i < n; i++) {
            float t = (float) i / n; // value from 0 to 1
            out[freq] += in[i] * cexp(2 * I * pi * freq * t); // 'Unmixing' the frequecies
        }
    }
}

int main()
{
    pi = atan2f(1, 1) * 4; // Fortran method to calc the PI

    const size_t N = 4096; // number of samples
    float in[N]; // input buffer (samples)
    float complex out[N]; // output buffer (frequencies)

    for (size_t i = 0; i < N; i++) {
        float t = (float) i / N;
        in[i] = cosf(2 * pi * t * 1) + sinf(2 * pi * t * 2); // Mixing 1 hertz and 2 hertz
    }

    //dft(in, out, N);
    fft(in, 1, out, N);

    /* for (size_t i = 0; i < N; i++) { */
    /*     printf("%zu: \t%5.2f, \t%5.2f\n", i, creal(out[i]), cimag(out[i])); */
    /* } */

    return 0;
}
