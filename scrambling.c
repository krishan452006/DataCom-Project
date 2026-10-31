#include "scrambling.h"

/* AMI: 0 becomes 0; each 1 alternates between +1 and -1. */
int encodeAMI(const char data[], int n, int signal[])
{
    int lastPulse = -1;

    for (int i = 0; i < n; i++) {
        if (data[i] == '0') {
            signal[i] = 0;
        } else {
            signal[i] = -lastPulse;
            lastPulse = signal[i];
        }
    }
    return n;
}

/* B8ZS replaces a run of eight 0s with a pattern containing violations.
   This implementation uses the standard pattern relative to the last pulse. */
int encodeB8ZS(const char data[], int n, int signal[])
{
    int lastPulse = -1;
    int i = 0;

    while (i < n) {
        int hasEightZeros = (i + 7 < n);
        for (int j = 0; hasEightZeros && j < 8; j++) {
            if (data[i + j] != '0') {
                hasEightZeros = 0;
            }
        }

        if (hasEightZeros) {
            signal[i] = 0;
            signal[i + 1] = 0;
            signal[i + 2] = 0;
            signal[i + 3] = lastPulse;
            signal[i + 4] = -lastPulse;
            signal[i + 5] = 0;
            signal[i + 6] = -lastPulse;
            signal[i + 7] = lastPulse;
            lastPulse = signal[i + 7];
            i += 8;
        } else {
            if (data[i] == '0') {
                signal[i] = 0;
            } else {
                signal[i] = -lastPulse;
                lastPulse = signal[i];
            }
            i++;
        }
    }
    return n;
}

/* HDB3 replaces four zeros. This teaching version shows the basic
   B00V / 000V substitution idea and keeps the same signal length. */
int encodeHDB3(const char data[], int n, int signal[])
{
    int lastPulse = -1;
    int pulseCount = 0;
    int i = 0;

    while (i < n) {
        int hasFourZeros = (i + 3 < n &&
            data[i] == '0' && data[i + 1] == '0' &&
            data[i + 2] == '0' && data[i + 3] == '0');

        if (hasFourZeros) {
            if (pulseCount % 2 == 0) {
                int balancingPulse = -lastPulse;
                signal[i] = balancingPulse;
                signal[i + 1] = 0;
                signal[i + 2] = 0;
                signal[i + 3] = balancingPulse; /* violation */
                lastPulse = balancingPulse;
            } else {
                signal[i] = 0;
                signal[i + 1] = 0;
                signal[i + 2] = 0;
                signal[i + 3] = lastPulse; /* violation */
            }
            pulseCount = 0;
            i += 4;
        } else {
            if (data[i] == '0') {
                signal[i] = 0;
            } else {
                signal[i] = -lastPulse;
                lastPulse = signal[i];
                pulseCount++;
            }
            i++;
        }
    }
    return n;
}
