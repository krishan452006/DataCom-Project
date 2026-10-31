DIGITAL SIGNAL ENCODING AND DECODING SIMULATOR
==============================================

Language:
- C (C11)

Files:
- main.c: menu and program flow
- data.c / data.h: random data, fixed zero runs, longest palindrome
- line_coding.c / line_coding.h: NRZ-L, NRZ-I, Manchester and Differential Manchester
- scrambling.c / scrambling.h: AMI, B8ZS and HDB3
- modulation.c / modulation.h: PCM and Delta Modulation
- signal_visualization.c / signal_visualization.h: text-based signal display

Compile (GCC / MinGW):
gcc -std=c11 -Wall -Wextra main.c data.c line_coding.c scrambling.c modulation.c signal_visualization.c -o simulator.exe

Run in PowerShell:
.\simulator.exe

GUI version (native C++ desktop interface):
1. Run build_gui.bat to compile the GUI app
2. Run run_gui.bat or double-click simulator_gui.exe

Notes:
- Enter binary input using only 0 and 1.
- For fixed-zero data, choose 4 or 8 zeros.
- Delta Modulation asks for analog sample values and a positive step size.
- PCM asks for analog sample values and bits per sample.
- The console version prints a text signal; the GUI plots rectangular signal levels,
  bit boundaries, and the 0 V reference line.
- Review and test the project before submission. Explain any AI assistance
  according to your course's academic integrity requirements.

Learning notes:
- NRZ-L maps each bit to a signal level.
- NRZ-I changes level when the input bit is 1.
- Manchester represents each bit using two signal levels.
- Differential Manchester always changes in the middle of each bit.
- AMI alternates the polarity of each 1.
- B8ZS and HDB3 replace long zero sequences with special pulse patterns.
- PCM quantizes analog values and represents each level with binary bits.
- Delta Modulation represents each sample as an increase or decrease.
