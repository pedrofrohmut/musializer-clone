#include <math.h>
#include <stdio.h>
#include <complex.h>

// 3.1415926535897932
#define PI 3.141592

float pi;

int main()
{
    pi = atan2f(1, 1) * 4;
    printf("PI: %f\n", pi);

    const size_t n = 8; // 8 samples
    float in[n]; // input buffer
    float complex out[n]; // store frequencies

    for (size_t i = 0; i < n; i++) {
        float t = (float) i / n;
        in[i] = cosf(2 * pi * t * 1) + sinf(2 * pi * t * 3); // Mixing 1 hertz and 3 hertz
    }

    for (size_t freq = 0; freq < n; freq++) {
        out[freq] = 0; // Accumulate the products
        for (size_t i = 0; i < n; i++) {
            float t = (float) i / n; // value from 0 to 1
            out[freq] += in[i] * cexp(2 * I * pi * freq * t); // 'Unmixing' the frequecies
        }
    }

    for (size_t i = 0; i < n; i++) {
        printf("%zu: \t%.2f, \t%.2f\n", i, creal(out[i]), cimag(out[i]));
    }

    return 0;
}
