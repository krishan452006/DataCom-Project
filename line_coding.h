#ifndef LINE_CODING_H
#define LINE_CODING_H

void encodeNRZL(const char data[], int n, int signal[]);
void decodeNRZL(const int signal[], int n, char decoded[]);
void encodeNRZI(const char data[], int n, int signal[]);
void decodeNRZI(const int signal[], int n, char decoded[]);
void encodeManchester(const char data[], int n, int signal[]);
void decodeManchester(const int signal[], int n, char decoded[]);
void encodeDifferentialManchester(const char data[], int n, int signal[]);
void decodeDifferentialManchester(const int signal[], int n, char decoded[]);
void printSignal(const int signal[], int n);

#endif
