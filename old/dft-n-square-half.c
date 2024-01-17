#include <math.h>
#include <stdio.h>
#include <complex.h>

// 3.1415926535897932
#define PI 3.141592

float pi;

/*
 A fast Fourier transform (FFT) is an algorithm that computes the discrete Fourier
 transform (DFT) of a sequence, or its inverse (IDFT). Fourier analysis converts a
 signal from its original domain (often time or space) to a representation in the
 frequency domain and vice versa. The DFT is obtained by decomposing a sequence of
 values into components of different frequencies.[1] This operation is useful in
 many fields, but computing it directly from the definition is often too slow to be
 practical. An FFT rapidly computes such transformations by factorizing the DFT
 matrix into a product of sparse (mostly zero) factors.[2] As a result, it manages
 to reduce the complexity of computing the DFT from O ( n 2 ) {\textstyle O(n^{2})},
 which arises if one simply applies the definition of DFT, to O ( n log ⁡ n ),
 where n is the data size. The difference in speed can be enormous, especially for
 long data sets where n may be in the thousands or millions. In the presence of
 round-off error, many FFT algorithms are much more accurate than evaluating the
 DFT definition directly or indirectly. There are many different FFT algorithms
 based on a wide range of published theories, from simple complex-number arithmetic
 to group theory and number theory.

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
int main()
{
    pi = atan2f(1, 1) * 4; // Fortran method to calc the PI

    const size_t N = 8; // number of samples

    float in[N]; // input buffer (samples)
    float complex out[N]; // output buffer (frequencies)

    for (size_t i = 0; i < N; i++) {
        float t = (float) i / N;
        in[i] = cosf(2 * pi * t * 1) + sinf(2 * pi * t * 2); // Mixing 1 hertz and 2 hertz
    }

    const size_t halfN = N / 2;

    // You just iterate half of the frequencies
    for (size_t freq = 0; freq < halfN; freq++) {
        out[freq] = 0;
        out[freq + halfN] = 0;

        // Even input positions
        for (size_t i = 0; i < N; i += 2) {
            const float t = (float) i / N;
            const float complex value = in[i] * cexp(2 * I * pi * t * freq);
            out[freq] += value;
            out[freq + halfN] += value;
        }

        // Odd input positions
        for (size_t i = 1; i <  N; i += 2) {
            const float t = (float) i / N;
            const float complex value = in[i] * cexp(2 * I * pi * t * freq);
            out[freq] += value;
            out[freq + halfN] -= value;
        }
    }

    for (size_t i = 0; i < N; i++) {
        printf("%zu: \t%5.2f, \t%5.2f\n", i, creal(out[i]), cimag(out[i]));
    }

    return 0;
}
