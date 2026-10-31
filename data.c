#include <stdio.h>
#include <stdlib.h>
#include "data.h"

/* Fill the array with random 0s and 1s. */
void generateRandomData(char data[], int n)
{
    for (int i = 0; i < n; i++) {
        data[i] = (rand() % 2) ? '1' : '0';
    }
    data[n] = '\0';
}

/* Generate random data, then put 4 or 8 zeros in one place. */
void generateDataWithZeros(char data[], int n, int zeroCount)
{
    generateRandomData(data, n);

    int start = rand() % (n - zeroCount + 1);
    for (int i = start; i < start + zeroCount; i++) {
        data[i] = '0';
    }
    data[n] = '\0';
}

/* Simple beginner-friendly palindrome search: O(n^2). */
void printLongestPalindrome(const char data[], int n)
{
    int bestStart = 0;
    int bestLength = 1;

    for (int start = 0; start < n; start++) {
        for (int end = start; end < n; end++) {
            int left = start;
            int right = end;
            int isPalindrome = 1;

            while (left < right) {
                if (data[left] != data[right]) {
                    isPalindrome = 0;
                    break;
                }
                left++;
                right--;
            }

            int length = end - start + 1;
            if (isPalindrome && length > bestLength) {
                bestStart = start;
                bestLength = length;
            }
        }
    }

    printf("Longest palindrome: ");
    for (int i = bestStart; i < bestStart + bestLength; i++) {
        printf("%c", data[i]);
    }
    printf("\nPalindrome length: %d\n", bestLength);
}
