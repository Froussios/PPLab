#!/bin/bash 

g++ -fopenmp openmp.cpp -o openmp
g++ sequential.cpp -o sequential

echo "All compiled"
