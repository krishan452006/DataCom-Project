#include <stdio.h>
#include "line_coding.h"

/* NRZ-L: 0 is +1 and 1 is -1. */
void encodeNRZL(const char data[], int n, int signal[])
{
    for (int i = 0; i < n; i++) {
        signal[i] = (data[i] == '0') ? 1 : -1;
    }
}

void decodeNRZL(const int signal[], int n, char decoded[])
{
    for (int i = 0; i < n; i++) {
        decoded[i] = (signal[i] == -1) ? '1' : '0';
    }
    decoded[n] = '\0';
}

/* NRZ-I: a 1 changes the level; a 0 keeps it the same. */
void encodeNRZI(const char data[], int n, int signal[])
{
    int level = 1;
    for (int i = 0; i < n; i++) {
        if (data[i] == '1') {
            level = -level;
        }
        signal[i] = level;
    }
}

void decodeNRZI(const int signal[], int n, char decoded[])
{
    int previousLevel = 1;
    for (int i = 0; i < n; i++) {
        decoded[i] = (signal[i] != previousLevel) ? '1' : '0';
        previousLevel = signal[i];
    }
    decoded[n] = '\0';
}

/* Manchester: each bit is represented by two signal levels. */
void encodeManchester(const char data[], int n, int signal[])
{
    for (int i = 0; i < n; i++) {
        if (data[i] == '0') {
            signal[2 * i] = -1;
            signal[2 * i + 1] = 1;
        } else {
            signal[2 * i] = 1;
            signal[2 * i + 1] = -1;
        }
    }
}

void decodeManchester(const int signal[], int n, char decoded[])
{
    for (int i = 0; i < n; i++) {
        decoded[i] = (signal[2 * i] == 1 && signal[2 * i + 1] == -1)
                     ? '1' : '0';
    }
    decoded[n] = '\0';
}

/* Differential Manchester always changes in the middle of each bit.
   A 0 changes at the beginning; a 1 does not. */
void encodeDifferentialManchester(const char data[], int n, int signal[])
{
    int level = 1;
    for (int i = 0; i < n; i++) {
        if (data[i] == '0') {
            level = -level;
        }
        signal[2 * i] = level;
        level = -level;
        signal[2 * i + 1] = level;
    }
}

void decodeDifferentialManchester(const int signal[], int n, char decoded[])
{
    int previousLevel = 1;
    for (int i = 0; i < n; i++) {
        decoded[i] = (signal[2 * i] != previousLevel) ? '0' : '1';
        previousLevel = signal[2 * i + 1];
    }
    decoded[n] = '\0';
}

void printSignal(const int signal[], int n)
{
    for (int i = 0; i < n; i++) {
        printf("%d ", signal[i]);
    }
    printf("\n");
}
