all: distances openmp

distances: distances.cpp
	g++ distances.cpp -pthread -o distances -DNDEBUG

openmp: openmp.cpp
	g++ openmp.cpp -fopenmp -o openmp -DNDEBUG
	g++ openmp.cpp -o openmpseq -DNDEBUG

clean:
	rm distances openmp openmpseq

