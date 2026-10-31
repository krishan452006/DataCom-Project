#include <stdio.h>
#include "signal_visualization.h"

/* Text-based view of the signal levels. */
void displayDigitalSignal(const int signal[], int n)
{
    printf("\nSignal view (one row per signal level):\n");
    for (int i = 0; i < n; i++) {
        printf("%3d | ", signal[i]);
        if (signal[i] > 0) {
            printf("HIGH  ++++++++\n");
        } else if (signal[i] < 0) {
            printf("LOW   --------\n");
        } else {
            printf("ZERO  ________\n");
        }
    }
    printf("\n");
}
