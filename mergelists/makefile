all: merge openmp

merge: merge.cpp
	g++ merge.cpp -pthread -o merge -DNDEBUG
#	g++ merge.cpp -pthread -o merge

openmp: openmp.cpp
	g++ openmp.cpp -fopenmp -o openmp 

clean:
	rm merge openmp
