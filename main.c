#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data.h"
#include "line_coding.h"
#include "scrambling.h"
#include "modulation.h"
#include "signal_visualization.h"

#define MAX_BITS 1000

static void showMenu(void)
{
    printf("\n====================================\n");
    printf(" DIGITAL SIGNAL ENCODING SIMULATOR\n");
    printf("====================================\n");
    printf("1. Generate random binary data\n");
    printf("2. Generate data with 4 or 8 consecutive zeros\n");
    printf("3. Enter binary data manually\n");
    printf("4. Encode/decode using NRZ-L\n");
    printf("5. Encode/decode using NRZ-I\n");
    printf("6. Encode/decode using Manchester\n");
    printf("7. Encode/decode using Differential Manchester\n");
    printf("8. Encode using AMI\n");
    printf("9. Encode using B8ZS\n");
    printf("10. Encode using HDB3\n");
    printf("11. Run Delta Modulation\n");
    printf("12. Run PCM\n");
    printf("0. Exit\n");
    printf("Choose an option: ");
}

static int readBinaryData(char data[], int *n)
{
    printf("Enter binary data (only 0 and 1, max %d bits): ", MAX_BITS);
    if (scanf("%1000s", data) != 1) {
        return 0;
    }

    *n = (int)strlen(data);
    if (*n < 1 || *n > MAX_BITS) {
        printf("Invalid data length.\n");
        return 0;
    }

    for (int i = 0; i < *n; i++) {
        if (data[i] != '0' && data[i] != '1') {
            printf("Only 0 and 1 are allowed.\n");
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    char data[MAX_BITS + 1];
    char decoded[MAX_BITS + 1];
    int signal[2 * MAX_BITS + 16];
    int choice;

    srand((unsigned int)time(NULL));

    while (1) {
        showMenu();

        if (scanf("%d", &choice) != 1) {
            printf("Please enter a number.\n");
            return 1;
        }

        if (choice == 0) {
            printf("Program closed.\n");
            break;
        }

        if (choice == 11) {
            runDeltaModulation();
            continue;
        }
        if (choice == 12) {
            runPCM();
            continue;
        }
        if (choice < 1 || choice > 10) {
            printf("Invalid option. Try again.\n");
            continue;
        }

        int n = 0;
        if (choice == 1 || choice == 2) {
            printf("Enter number of bits (1-%d): ", MAX_BITS);
            if (scanf("%d", &n) != 1 || n < 1 || n > MAX_BITS) {
                printf("Invalid number of bits.\n");
                continue;
            }

            if (choice == 1) {
                generateRandomData(data, n);
            } else {
                int zeros;
                printf("How many consecutive zeros? Enter 4 or 8: ");
                if (scanf("%d", &zeros) != 1 ||
                    (zeros != 4 && zeros != 8) || zeros > n) {
                    printf("Please enter 4 or 8, not more than the data length.\n");
                    continue;
                }
                generateDataWithZeros(data, n, zeros);
            }
        } else {
            if (!readBinaryData(data, &n)) {
                continue;
            }
        }

        printf("\nInput data: %s\n", data);
        printLongestPalindrome(data, n);

        if (choice <= 3) {
            continue;
        }

        int signalLength = n;

        if (choice == 4) {
            encodeNRZL(data, n, signal);
            decodeNRZL(signal, n, decoded);
        } else if (choice == 5) {
            encodeNRZI(data, n, signal);
            decodeNRZI(signal, n, decoded);
        } else if (choice == 6) {
            encodeManchester(data, n, signal);
            signalLength = 2 * n;
            decodeManchester(signal, n, decoded);
        } else if (choice == 7) {
            encodeDifferentialManchester(data, n, signal);
            signalLength = 2 * n;
            decodeDifferentialManchester(signal, n, decoded);
        } else if (choice == 8) {
            signalLength = encodeAMI(data, n, signal);
        } else if (choice == 9) {
            signalLength = encodeB8ZS(data, n, signal);
        } else if (choice == 10) {
            signalLength = encodeHDB3(data, n, signal);
        }

        printf("Encoded signal levels: ");
        printSignal(signal, signalLength);
        displayDigitalSignal(signal, signalLength);

        if (choice >= 4 && choice <= 7) {
            printf("Decoded data: %s\n", decoded);
            if (strcmp(data, decoded) == 0) {
                printf("Check: original data was recovered correctly.\n");
            } else {
                printf("Check: decoded data does not match the input.\n");
            }
        }
    }

    return 0;
}
