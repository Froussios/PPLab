all: distances openmp

distances: distances.cpp
	g++ distances.cpp -pthread -o distances -DNDEBUG

openmp: openmp.cpp
	g++ openmp.cpp -fopenmp -o openmp -DNDEBUG

clean:
	rm distances openmp

