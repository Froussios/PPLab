all: minima openmp

minima: minima.cpp
	g++ minima.cpp -pthread -o minima -DNDEBUG

openmp: openmp.cpp
	g++ openmp.cpp -fopenmp -o openmp -DNDEBUG
	g++ openmp.cpp -o openmpseq -DNDEBUG

clean:
	rm openmp minima openmpseq

