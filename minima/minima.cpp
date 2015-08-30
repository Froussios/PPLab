#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <sys/time.h>
#include <assert.h>

using namespace std;

// Number of threads
#define NUM_THREADS 32 // TODO 32

// Number of iterations
#define TIMES 100

// Input Size
#define NSIZE 7
#define NMAX 262144
int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};    

typedef struct __ThreadArg {
  int id;
  int nrT;
  int n;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t completed_barr, internal_barr, phase_barr;

// Seed Input
int A[NMAX];

int expand[NMAX*2];
int reduce[NMAX*2];

// Output 
int prefix[NMAX];
int suffix[NMAX];


// Calculate the index of item on a binary tree,
// if the tree is stored on an array
int index(int height, int i) {
	return (1 << height) - 1 + i;
}

// Same as index, only the items on the same level
// are traversed in reverse order (right to left)
int index_rev(int height, int i) {
	return (2 << height) - 2 - i;
}

int min(int a, int b) {
	return (a>b) ? b : a;
}


void init(int n){
	// Populate the base of the tree
	int logn = ceil(log2(n));
	for (int i=0 ; i<n ; i++)
		expand[index(logn, i)] = A[i];
}

string toString(int a[], int n) {
	ostringstream oss;
	for (int i=0 ; i<n ; i++)
		oss << a[i] << ",";
	return oss.str();
}

// Perform simple tests to verify correct execution
void testResult(int n) {
	#ifndef NDEBUG
	bool pass;
	
	pass = true;
	for (int i=1 ; i<n ; i++)
		if (prefix[i-1] < prefix[i])
			pass = false;
	if (!pass) {
		cerr << "[ERROR 1]" << endl;
		cerr << "[" << toString(expand, n) << "]" << endl;
		cerr << "[" << toString(prefix, n) << "]" << endl;
	}
	
	pass = true;
	for (int i=1 ; i<n ; i++)
		if (suffix[i-1] > suffix[i])
			pass = false;
	if (!pass) {
		cerr << "[ERROR 2]" << endl;
		cerr << "[" << toString(expand, n) << "]" << endl;
		cerr << "[" << toString(suffix, n) << "]" << endl;
	}
	
	#endif
}

void seq_function(int n){
	int leveln = n;
	int height;	
	int logn = ceil(log2(n));
	
	#ifndef NDEBUG
	//for (int i=0 ; i<n*2 ; i++)
	//	expand[i] = reduce[i] = -1;
	#endif
		
	// Construct minima tree
	
	for (int h=logn-1 ; h>=0 ; h--) {
		leveln /= 2;			
		for (int i=0 ; i<leveln ; i++) {
			expand[index(h, i)] = min(expand[index(h+1, 2*i)], expand[index(h+1, 2*i+1)]);
		}
	}
	
	// Prefix minima
	
	leveln = 1;
	for (int h=0 ; h<=logn ; h++) {
		assert(leveln < 2*n);
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
	for (int i=0 ; i<n ; i++)
		prefix[i] = reduce[index(logn, i)];
		
		
	// Suffix minima
	
	leveln = 1;
	for (int h=0 ; h<=logn ; h++) {
		assert(leveln < 2*n);
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
	for (int i=0 ; i<n ; i++)
		suffix[i] = reduce[index(logn, i)];
}

// Determine boundaries for this thread
int boundary_first(int rank, int numThreads, int size) {
	if (numThreads <= size) // If more data than threads, make chunks
		return rank * (size / numThreads);
	else if (rank < size)   // else, one item per thread
		return rank;
	else                    // and some threads get nothing
		return 0;
}
int boundary_last(int rank, int numThreads, int size) {
	if (numThreads <= size)
		return (rank == numThreads-1) ? 
			size : (rank + 1) * (size / numThreads);
	else if (rank < size)
		return rank + 1;
	else
		return 0;
}

void* par_function(void* a) {
	tThreadArg *args = (tThreadArg*) a;
	int threadRank = args->id - 1;
	int threadCount = args->nrT;
	int n = args->n;
	
	int first, last; // The range that this thread will work on

	for (int t=0 ; t<TIMES ; t++) {
		// Wait for initialisation to complete
		pthread_barrier_wait(&internal_barr);
		
		int leveln = n;
		int height;	
		int logn = ceil(log2(n));
	
		#ifndef NDEBUG
		//for (int i=0 ; i<n*2 ; i++)
		//	expand[i] = reduce[i] = -1;
		#endif
		
		// Construct minima tree
		for (int h=logn-1 ; h>=0 ; h--) {
			leveln /= 2;
			
			first = boundary_first(threadRank, threadCount, leveln);
			last = boundary_last(threadRank, threadCount, leveln);
			for (int i=first ; i<last ; i++) {
				expand[index(h, i)] = min(expand[index(h+1, 2*i)], expand[index(h+1, 2*i+1)]);
			}
			
			// Wait for this level of the tree to complete
			pthread_barrier_wait(&phase_barr);
		}
	
		// Prefix minima
		leveln = 1;
		for (int h=0 ; h<=logn ; h++) {
			assert(leveln < 2*n);
			
			first = boundary_first(threadRank, threadCount, leveln);
			last = boundary_last(threadRank, threadCount, leveln);
			for (int i=first ; i<last ; i++) {				
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
			
			// Wait for this level of the tree to complete
			pthread_barrier_wait(&phase_barr);
		}
	
		// Copy result to result container
		first = boundary_first(threadRank, threadCount, n);
		last = boundary_last(threadRank, threadCount, n);
		for (int i=first ; i<last ; i++)
			prefix[i] = reduce[index(logn, i)];
		
		// Finish with prefix before doing suffix
		pthread_barrier_wait(&phase_barr);
		
		// Suffix minima
		leveln = 1;
		for (int h=0 ; h<=logn ; h++) {
			assert(leveln < 2*n);
			
			first = boundary_first(threadRank, threadCount, leveln);
			last = boundary_last(threadRank, threadCount, leveln);
			for (int i=first ; i<last ; i++) {				
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
			
			// Wait for this level of the tree to complete
			pthread_barrier_wait(&phase_barr);			
		}
		
		// Copy result to result container
		first = boundary_first(threadRank, threadCount, n);
		last = boundary_last(threadRank, threadCount, n);
		for (int i=first ; i<last ; i++)
			suffix[i] = reduce[index(logn, i)];
	}
	
	pthread_barrier_wait(&completed_barr);
	pthread_exit(0);
}


// Instead of speedtests, show a readable correctness test
int presentCorrectness() {
	const int N = 16;
	const int nt = 4;
	void *status;
   	pthread_attr_t attr;
  	tThreadArg x[NUM_THREADS];
	
	srand ( time(NULL) );
	for(int k=0; k<N; k++){
		A[k] = rand() % 100; // Readable numbers
	}
	
	cout << "Input: " << toString(A, N) << endl;
	
	cout << "Sequential:" << endl;
	init(N);
	seq_function(N);
	cout << "Prefix: " << toString(prefix, N) << endl;
	cout << "Suffix: " << toString(suffix, N) << endl;
	
	
	cout << "Parallel: 4 threads" << endl;
	
	/* Initialize and set thread detached attribute */
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if(pthread_barrier_init(&completed_barr, NULL, nt+1) ||
			pthread_barrier_init(&internal_barr, NULL, nt+1) ||
			pthread_barrier_init(&phase_barr, NULL, nt)) {
		printf("Could not create a barrier\n");
        return -1;
	}

	for (int j=1; j<=nt; j++)
		{
		x[j].id = j; 
		x[j].nrT=nt; // number of threads in this round
		x[j].n=N;  //input size
		pthread_create(&callThd[j-1], &attr, par_function, (void *)&x[j]);
	}

	for (int t=0; t<TIMES; t++) // Repetitions are built into the parallel function
	{
		init(N);
		pthread_barrier_wait(&internal_barr);
	}
	pthread_barrier_wait(&completed_barr);

	/* Wait on the other threads */
	for(int j=0; j<nt; j++)
		pthread_join(callThd[j], &status);

	if (pthread_barrier_destroy(&completed_barr) ||
			pthread_barrier_destroy(&internal_barr) ||
			pthread_barrier_destroy(&phase_barr)) {
		printf("Could not destroy the barrier\n");
        return -1;
	}
	
	cout << "Prefix: " << toString(prefix, N) << endl;
	cout << "Suffix: " << toString(suffix, N) << endl;	
}

int main (int argc, char *argv[])
{
  	struct timeval startt, endt, result;
	int i, j, k, nt, t, n, c;
	void *status;
   	pthread_attr_t attr;
  	tThreadArg x[NUM_THREADS];
  	
  	if (argc > 1 && string(argv[1]) == "correctness") {
  		return presentCorrectness();
  	}
  		
	
  	result.tv_sec = 0;
  	result.tv_usec= 0;

	/* Generate a seed input */
	srand ( time(NULL) );
	for(k=0; k<NMAX; k++){
		A[k] = rand();
	}
	

   	/* Initialize and set thread detached attribute */
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	printf("|NSize|Iterations|Seq|Th01|Th02|Th04|Th08|Par16|\n");

	// for each input size
	for(c=0; c<NSIZE; c++){
		n=Ns[c];		
		printf("| %d | %d |",n,TIMES);
		
		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		
		testResult(n);
			
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){
	        if(pthread_barrier_init(&completed_barr, NULL, nt+1))
			{
    			printf("Could not create a barrier\n");
		        return -1;
			}
	        if(pthread_barrier_init(&internal_barr, NULL, nt+1))
			{
    			printf("Could not create a barrier\n");
		        return -1;
			}
			if(pthread_barrier_init(&phase_barr, NULL, nt))
			{
    			printf("Could not create a barrier\n");
		        return -1;
			}

			result.tv_sec=0; result.tv_usec=0;
			for (j=1; j<=/*NUMTHRDS*/nt; j++)
        		{
				x[j].id = j; 
				x[j].nrT=nt; // number of threads in this round
				x[j].n=n;  //input size
				pthread_create(&callThd[j-1], &attr, par_function, (void *)&x[j]);
			}

			gettimeofday (&startt, NULL);
			for (t=0; t<TIMES; t++) 
			{
				init(n);
				pthread_barrier_wait(&internal_barr);
			}
			pthread_barrier_wait(&completed_barr);
			gettimeofday (&endt, NULL);
			
			testResult(n);

			/* Wait on the other threads */
			for(j=0; j</*NUMTHRDS*/nt; j++)
			{
				pthread_join(callThd[j], &status);
			}

			if (pthread_barrier_destroy(&completed_barr)) {
        			printf("Could not destroy the barrier\n");
			        return -1;
			}
			if (pthread_barrier_destroy(&internal_barr)) {
        			printf("Could not destroy the barrier\n");
			        return -1;
			}
			if (pthread_barrier_destroy(&phase_barr)) {
        			printf("Could not destroy the barrier\n");
			        return -1;
			}
   			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);
		}
		printf("\n");
	}
	pthread_exit(NULL);
}

