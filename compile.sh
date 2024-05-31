#!/bin/bash
g++ -Ofast -std=c++20 -march=native -mtune=native -funroll-loops -finline-functions -o heuristic main.cpp
g++ -Ofast -std=c++20 -march=native -mtune=native -funroll-loops -finline-functions -D EXACT -o exact main.cpp
