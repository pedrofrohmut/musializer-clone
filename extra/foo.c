#include <math.h>
#include <complex.h>
#include  <stdio.h>
#include <assert.h>

# define M_PI		3.14159265358979323846	/* pi */
#define N (30)

float in[N];
float complex out[N];

void dft(float in[], float complex out[], size_t n)
{
    for (size_t freq = 0; freq < n; freq++) {
        out[freq] = 0; // Accumulate the products
        for (size_t i = 0; i < n; i++) {
            float t = (float) i / n; // value from 0 to 1
            out[freq] += in[i] * cexp(2 * I * M_PI * freq * t); // 'Unmixing' the frequecies
        }
    }
}

// 1. When the freq is not an integer value the phantom freqs start to come up
// 2. One way to get rid of this phantom freqs is to apply the windowing function
int main(void)
{
    assert(N > 0);
    /* for (size_t i = 0; i < N; i++) { */
    /*     // Hann function */
    /*     //float my_hann = 0.5 - (0.5 * cos((2 * M_PI * N) / (N - 1))); */
    /*  */
    /*     // Hann function */
    /*     float t = (float) i / (N - 1); */
    /*     float hann = 0.5 - 0.5 * cosf(2 * M_PI * t); */
    /*     //printf("%f\n", hann); */
    /*  */
    /*     float M = hann * 100; */
    /*     for (size_t j = 0; j < M; j++) { */
    /*         printf("."); */
    /*     } */
    /*     printf("#\n"); */
    /*  */
    /* } */
    /*  */
    /* return 0; // CODE BLOCKER */

    float freq = 2.0f;
    for (size_t i = 0; i < N; i++) {
        float t = (float) i / (N - 1);
#if 1
        float hann = 0.5 - 0.5 * cosf(2 * M_PI * t);
        in[i] = hann * sinf(2 * M_PI * t * freq);
#else
        in[i] = sinf(2 * M_PI * t * freq);
#endif
    }

    dft(in, out, N);

    float max = 0;
    for (size_t i = 0; i < N; i++) {
        float x = cabsf(out[i]);
        if (x > max) max = x;
    }

    /* for (size_t i = 0; i < N; i++) { */
    /*     float x = cabsf(out[i]); */
    /*     printf("%f\n", x / max); */
    /* } */

    for (size_t i = 0; i < N / 2; i++) {
        float x = cabsf(out[i]);
        /* float t = (in[i] + 1) / 2; */
        float tmp = x / max;
        for (size_t j = 0; j < tmp * N; j++) {
            printf(".");
        }
        printf("#\n");
    }

    printf("--------------------------------------------------------------------------------\n");

#if 1
    size_t repeat = 2;
    for (size_t k = 0; k < repeat; k++) {
        for (size_t i = 0; i < N; i++) {
            float t = (in[i] + 1) / 2;
            for (size_t j = 0; j < t * N; j++) {
                printf(".");
            }
            printf("#\n");
        }
    }
#endif

    return 0;
}

#if 0
int main(void)
{
    for (size_t i = 0; i < N; i++) {
        float t = (float) i / N;
        in[i] = sinf(2 * M_PI * t);
    }

    for (size_t i = 0; i < N; i++) {
        float width = in[i] * 10;
        if (in[i] < 0) {
            width = 10 + width;
        } else {
            width += 10;
        }
        for (size_t j = 0; j < width; j++) {
            printf("-");
        }
        printf("#\n");
    }

    return 0;
}
#endif
