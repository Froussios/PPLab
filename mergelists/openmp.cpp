#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iterator>
#include <ctime>
#include <assert.h>

// Number of threads
#define NUM_THREADS 8

// Number of iterations
#define TIMES 100

// Input Size
#define NSIZE 7
#define NMAX 262144
int ANs[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};   
int BNs[NSIZE] = {1024, 2048, 4096, 8192, 16384, 32768, 65536};


int A[NMAX];
int B[NMAX];
int AB[2*NMAX];


using namespace std;

class stopwatch {
	unsigned int start, end;
public:
	stopwatch() { restart(); }
	void restart() { start = clock(); }
	void stop() { end = clock(); }
	unsigned int elapsed() { return end - start; }
};

// calculate rank(item : a)
int binaryRank(int a[], int start, int end, int item) {
	if (start == end - 1)
		if (a[start] <= item)
			return start + 1;
		else
			return start;

	int mid = (start + end) / 2;
	if (item >= a[mid])
		return binaryRank(a, mid, end, item);
	else
		return binaryRank(a, start, mid, item);
}

#ifndef NDEBUG
bool isIncreasing(int a[], int n) {
	for (int i=0 ; i<n-1 ; i++)
		if (v[i] >= v[i+1])
			return false;
	return true;
}
#endif


int main(int argc, char *args[])
{
	stopwatch sw;
	int i;

	// Read arguments
	string infile("data.txt"), outfile("output.txt");
	for (int i = 0; i < argc; i++) {
		string arg(args[i]);
		if (arg == "-i")
			infile = args[++i];
		if (arg == "-o")
			outfile = args[++i];
	}

	/* Generate input */
	// Generate two sorted lists with no common items
	srand ( time(NULL) );
	int ia=0, ib=0;
	int number = rand() % 5;
	while (ia < NMAX && ib < NMAX)
		if (rand() % 5 > 0) //A is 4x larger than B 
			A[ia++] = number = number + 1 + (rand() % 5);
		else
			B[ib++] = number = number + 1 + (rand() % 5);
	while (ia < NMAX)
		A[ia++] = number = number + 1 + (rand() % 5);
	while (ib < NMAX)
		B[ib++] = number = number + 1 + (rand() % 5);

	#ifndef NDEBUG
	assert(isIncreasing(A));
	assert(isIncreasing(B));
	#endif
	
	cout << " N-M | # | 1 thread | 2 threads | 4 threads | 8 threads |" << endl;

	for (int c=0 ; c<NSIZE ; c++) {
		const int an = ANs[c];
		const int bn = ANs[c];
		
		cout << an << "-" << bn << " | " << TIMES << " | ";
		
		
		for (int nthreads=1 ; nthreads <= NUM_THREADS ; nthreads<<=1) {
			sw.restart();
	
			// Do merge
			#pragma omp parallel num_threads(nthreads) shared(A, B, AB) private(i)
			{
				for (int t=0 ; t<TIMES ; t++) {
					// Calculate (A:AB)
					#pragma omp for schedule(static) nowait
					for (i = 0; i < an; i++) {
						int rankAB = i + 1 + binaryRank(B, 0, bn, A[i]);
						AB[rankAB - 1] = A[i];
					}

					// Calculate (B:AB)
					#pragma omp for schedule(static) nowait
					for (i = 0; i < bn; i++) {
						int rankAB = i + 1 + binaryRank(A, 0, an, B[i]);
						AB[rankAB - 1] = B[i];
					}
				}
			}
		
			sw.stop();
			
			cout << sw.elapsed() << " | ";
		}
		
		cout << endl;
	}

	

	#ifndef NDEBUG
	assert(isIncreasing(AB));
	#endif


	#ifdef _WIN32
	string dummy;
	getline(cin, dummy);
	#endif
    return 0;
}

