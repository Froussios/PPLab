#include <omp.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <assert.h>

// Number of threads
#define NUM_THREADS 8 // TODO 32

// Number of iterations
#define TIMES 100

// Input Size
#define NSIZE 7
#define NMAX 262144
int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};

// Seed Input
int A[NMAX];

int expand[NMAX*2];
int reduce[NMAX*2];

// Output 
int prefix[NMAX];
int suffix[NMAX];
 

using namespace std;


class stopwatch {
	unsigned int start, end;
public:
	stopwatch() { restart(); }
	void restart() { start = clock(); }
	void stop() { end = clock(); }
	unsigned int elapsed() { return end - start; }
};

int index(int height, int i) {
	return (1 << height) - 1 + i;
}

int index_rev(int height, int i) {
	return (2 << height) - 2 - i;
}

int min(int a, int b) {
	return (a>b) ? b : a;
}


string arrayToString(int data[], int dataCount) {
	stringstream ss;
	for (int i = 0; i < dataCount; i++) {
		ss << setfill(' ') << setw(4) << data[i] << " ";
	}
	return ss.str();
}


main ()
{
	stopwatch sw;
	int nthreads;
	int i;
	
	cout << "N | # | 1 threads | 2 threads | 4 threads | 8 threads " << endl;
	
	for (int c=0 ; c<NSIZE ; c++) {	
		int n = Ns[c];
		
		cout << n << " | " << TIMES << " | ";
		
		for (int nthreads=1 ; nthreads<=NUM_THREADS ; nthreads <<= 1) {
			
			sw.restart();
			
			#pragma omp parallel shared(reduce, expand) private(i)
			{
				for (int t=0 ; t<TIMES ; t++) {
					int leveln = n;
					int height;	
					int logn = ceil(log2(n));
	
					#ifndef NDEBUG
					for (int i=0 ; i<n*2 ; i++)
						expand[i] = reduce[i] = -1;
					#endif
		
					// Construct minima tree
	
					for (int h=logn-1 ; h>=0 ; h--) {
						leveln /= 2;
						#pragma omp for	
						for (int i=0 ; i<leveln ; i++) {
							expand[index(h, i)] = min(expand[index(h+1, 2*i)], expand[index(h+1, 2*i+1)]);
						}
					}
	
					// Prefix minima
	
					leveln = 1;
					for (int h=0 ; h<=logn ; h++) {
						assert(leveln < 2*n);
						
						#pragma omp for	
						for (int i=0 ; i<leveln ; i++) {				
							if (i == 0)
								reduce[index(h, i)] = expand[index(h, i)];
							else if (i % 2 == 1)
								reduce[index(h, i)] = reduce[index(h-1, i/2)];
							else {
								if (expand[index(h, i+1)] != reduce[index(h-1, i/2)])
									reduce[index(h, i)] = reduce[index(h-1, i/2)];
								else
									reduce[index(h, i)] = reduce[index(h, i-1)];
							}
						}
						leveln *= 2;
					}
	
					// Copy to result container
					#pragma omp for	
					for (int i=0 ; i<n ; i++)
						prefix[i] = reduce[index(logn, i)];
		
		
					// Suffix minima
	
					leveln = 1;
					for (int h=0 ; h<=logn ; h++) {
						assert(leveln < 2*n);
						
						#pragma omp for	
						for (int i=0 ; i<leveln ; i++) {				
							if (i == 0)
								reduce[index_rev(h, i)] = expand[index_rev(h, i)];
							else if (i % 2 == 1)
								reduce[index_rev(h, i)] = reduce[index_rev(h-1, i/2)];
							else {
								if (expand[index_rev(h, i+1)] != reduce[index_rev(h-1, i/2)])
									reduce[index_rev(h, i)] = reduce[index_rev(h-1, i/2)];
								else
									reduce[index_rev(h, i)] = reduce[index_rev(h, i-1)];
							}
						}
						leveln *= 2;
					}
	
					// Copy to result container
					#pragma omp for	
					for (int i=0 ; i<n ; i++)
						suffix[i] = reduce[index(logn, i)];
				}
			}
			
			sw.stop();
		
			cout << sw.elapsed() << " | ";	
		}
		
		cout << endl;		
	}

#ifdef _WIN32
	string dummy;
	getline(cin, dummy);
#endif

    return 0;
}
