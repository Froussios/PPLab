all: openmp sequential pthreads 

openmp: openmp.cpp
	g++ openmp.cpp -fopenmp -o openmp

sequential: sequential.cpp
	g++ sequential.cpp -o sequential

pthreads: pthreads.cpp
	g++ pthreads.cpp -o pthreads

clean:
	rm openmp sequential pthreads

