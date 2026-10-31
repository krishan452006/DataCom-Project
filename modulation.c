#include <stdio.h>
#include <math.h>
#include "modulation.h"

#define MAX_SAMPLES 100

/* Delta Modulation: encode each sample as up (1) or down (0),
   then rebuild a staircase from the bits. */
void runDeltaModulation(void)
{
    int n;
    double step;
    double samples[MAX_SAMPLES];
    double reconstructed = 0.0;
    int bits[MAX_SAMPLES];

    printf("\n--- Delta Modulation ---\n");
    printf("Number of analog samples (1-%d): ", MAX_SAMPLES);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_SAMPLES) {
        printf("Invalid sample count.\n");
        return;
    }

    printf("Positive step size: ");
    if (scanf("%lf", &step) != 1 || step <= 0) {
        printf("Step size must be positive.\n");
        return;
    }

    printf("Enter %d analog sample values:\n", n);
    for (int i = 0; i < n; i++) {
        if (scanf("%lf", &samples[i]) != 1) {
            printf("Invalid sample.\n");
            return;
        }
    }

    printf("Encoded bits: ");
    for (int i = 0; i < n; i++) {
        if (samples[i] >= reconstructed) {
            bits[i] = 1;
            reconstructed += step;
        } else {
            bits[i] = 0;
            reconstructed -= step;
        }
        printf("%d", bits[i]);
    }

    reconstructed = 0.0;
    printf("\nDecoded staircase: ");
    for (int i = 0; i < n; i++) {
        reconstructed += (bits[i] == 1) ? step : -step;
        printf("%.2f ", reconstructed);
    }
    printf("\n");
}

/* PCM: map samples to integer levels, print binary codes, and decode
   each code to the centre of its quantization interval. */
void runPCM(void)
{
    int n, bitsPerSample;
    double samples[MAX_SAMPLES];
    double decoded[MAX_SAMPLES];
    int codes[MAX_SAMPLES];
    double minValue, maxValue, step;
    int levels;

    printf("\n--- Pulse Code Modulation (PCM) ---\n");
    printf("Number of analog samples (1-%d): ", MAX_SAMPLES);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_SAMPLES) {
        printf("Invalid sample count.\n");
        return;
    }

    printf("Bits per sample (1-8): ");
    if (scanf("%d", &bitsPerSample) != 1 ||
        bitsPerSample < 1 || bitsPerSample > 8) {
        printf("Enter a value from 1 to 8.\n");
        return;
    }

    printf("Enter %d analog sample values:\n", n);
    for (int i = 0; i < n; i++) {
        if (scanf("%lf", &samples[i]) != 1) {
            printf("Invalid sample.\n");
            return;
        }
    }

    minValue = maxValue = samples[0];
    for (int i = 1; i < n; i++) {
        if (samples[i] < minValue) minValue = samples[i];
        if (samples[i] > maxValue) maxValue = samples[i];
    }

    levels = 1 << bitsPerSample;
    step = (maxValue - minValue) / levels;

    printf("PCM codes: ");
    for (int i = 0; i < n; i++) {
        if (step == 0.0) {
            codes[i] = 0;
            decoded[i] = minValue;
        } else {
            codes[i] = (int)((samples[i] - minValue) / step);
            if (codes[i] >= levels) codes[i] = levels - 1;
            if (codes[i] < 0) codes[i] = 0;
            decoded[i] = minValue + (codes[i] + 0.5) * step;
        }

        for (int bit = bitsPerSample - 1; bit >= 0; bit--) {
            printf("%d", (codes[i] >> bit) & 1);
        }
        printf(" ");
    }

    printf("\nDecoded samples: ");
    for (int i = 0; i < n; i++) {
        printf("%.2f ", decoded[i]);
    }
    printf("\n");
}
