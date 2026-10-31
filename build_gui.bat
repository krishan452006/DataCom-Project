@echo off
cd /d "%~dp0"
gcc -c data.c line_coding.c scrambling.c modulation.c
g++ -std=c++17 -c main_gui.cpp
g++ -std=c++17 -mwindows main_gui.o data.o line_coding.o scrambling.o modulation.o -lgdi32 -o simulator_gui.exe
if exist simulator_gui.exe echo GUI build complete.
