all: merge openmp openmpseq

merge: merge.cpp
	g++ merge.cpp -pthread -o merge -DNDEBUG
#	g++ merge.cpp -pthread -o merge

openmp: openmp.cpp
	g++ openmp.cpp -fopenmp -o openmp -DNDEBUG
	g++ openmp.cpp -o openmpseq -DNDEBUG

clean:
	rm merge openmp openmpseq
