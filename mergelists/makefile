all: merge openmp

merge: merge.cpp
	g++ merge.cpp -pthread -o merge -DNDEBUG

openmp: openmp.cpp
	g++ openmp.cpp -fopenmp -o openmp -DNDEBUG
	g++ openmp.cpp -o openmpseq -DNDEBUG

clean:
	rm merge openmp openmpseq
