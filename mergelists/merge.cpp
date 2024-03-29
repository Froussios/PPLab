#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <assert.h>

using namespace std;

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 100

// Input Size
#define NSIZE 7
#define NMAX 262144
int ANs[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};   
int BNs[NSIZE] = {1024, 2048, 4096, 8192, 16384, 32768, 65536};   

typedef struct __ThreadArg {
  int id;
  int nrT;
  int nA;
  int nB;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t completed_barr, internal_barr;


int A[NMAX];
int B[NMAX];
int AB[2*NMAX];

// For printing arrays
string toString(int a[], int n) {
	ostringstream oss;
	for (int i=0 ; i<n ; i++)
		oss << a[i] << ",";
	return oss.str();
}

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

// Initialize subset
void init(int na, int nb){
	
	#ifndef NDEBUG
	// Initialise all tables
	for (int i=0 ; i<NMAX ; i++)
		AB[i] = AB[i*2] = -1;
	#endif
}

void seq_function(int sizeA, int sizeB){
	int i;

	for (i = 0; i < sizeA; i++) {
		// Calculate (A[i]:AB)
		int A_AB_i = i + 1 + binaryRank(B, 0, sizeB, A[i]);
		// put A[i] in AB
		AB[A_AB_i - 1] = A[i];
	}

	for (i = 0; i < sizeB; i++) {
		// Calculate (B[i]:AB)
		int B_AB_i = i + 1 + binaryRank(A, 0, sizeA, B[i]);
		// put B[i] in AB
		AB[B_AB_i - 1] = B[i];
	}
}

void* par_function(void* a){
	/* The code for threaded computation */
	// Perform operations on B
	
	tThreadArg *args = (tThreadArg*) a;
	int threadRank = args->id - 1;
	int threadCount = args->nrT;
	int sizeA = args->nA;
	int sizeB = args->nB;
	int i;
	
	for (int t=0 ; t<TIMES ; t++) {
	
		// Wait for data to be initialised for this iteration
		pthread_barrier_wait(&internal_barr);
		
		// Determine boundaries for this thread
		int firstItemA = threadRank * (sizeA / threadCount);
		int firstItemB = threadRank * (sizeB / threadCount);
		int lastItemA = (threadRank == threadCount-1) ? 
							sizeA : (threadRank + 1) * (sizeA / threadCount);
		int lastItemB = (threadRank == threadCount-1) ?
							sizeB : (threadRank + 1) * (sizeB / threadCount);
	
		// Calculate (A:AB)
		for (i = firstItemA; i < lastItemA; i++) {
			int A_AB_i = i + 1 + binaryRank(B, 0, sizeB, A[i]);
			AB[A_AB_i - 1] = A[i];
		}

		// Calculate (B:AB)
		for (i = firstItemB; i < lastItemB; i++) {
			int B_AB_i = i + 1 + binaryRank(A, 0, sizeA, B[i]);
			AB[B_AB_i - 1] = B[i];
		}
		
		// Iteration completed
		pthread_barrier_wait(&completed_barr);
	}
	
	pthread_exit(0);
}

// Instead of speedtests, show a readable correctness test
int presentCorrectness() {
	const int AN = 32;
	const int BN = 16;
	const int nt = 4;
	void *status;
   	pthread_attr_t attr;
  	tThreadArg x[NUM_THREADS];
	
	/* Generate input */
	// Generate two sorted lists with no common items
	srand ( time(NULL) );
	int ia=0, ib=0;
	int number = rand() % 5;
	while (ia < AN && ib < BN)
		if (rand() % 5 > 0) //A is 4x larger than B 
			A[ia++] = number = number + 1 + (rand() % 5);
		else
			B[ib++] = number = number + 1 + (rand() % 5);
	while (ia < AN)
		A[ia++] = number = number + 1 + (rand() % 5);
	while (ib < BN)
		B[ib++] = number = number + 1 + (rand() % 5);
	
	cout << "Input A: " << toString(A, AN) << endl;
	cout << "Input A: " << toString(B, BN) << endl;
	
	cout << "Sequential:" << endl;
	init(AN, BN);
	seq_function(AN, BN);
	cout << "AB: " << toString(AB, AN+BN) << endl;
	
	
	cout << "Parallel: 4 threads" << endl;
	
	/* Initialize and set thread detached attribute */
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if(pthread_barrier_init(&completed_barr, NULL, nt+1) ||
			pthread_barrier_init(&internal_barr, NULL, nt+1)) {
		printf("Could not create a barrier\n");
        return -1;
	}

	for (int j=1; j<=nt; j++) {
		x[j].id = j; 
		x[j].nrT=nt; // number of threads in this round
		x[j].nA=AN;  //input size
		x[j].nB=BN;
		pthread_create(&callThd[j-1], &attr, par_function, (void *)&x[j]);
	}

	for (int t=0; t<TIMES; t++) // Repetitions are built into the parallel function
	{
		init(AN, BN);
		pthread_barrier_wait(&internal_barr);
		pthread_barrier_wait(&completed_barr);
	}

	/* Wait on the other threads */
	for(int j=0; j<nt; j++)
		pthread_join(callThd[j], &status);

	if (pthread_barrier_destroy(&completed_barr) ||
			pthread_barrier_destroy(&internal_barr)) {
		printf("Could not destroy the barrier\n");
        return -1;
	}
	
	cout << "AB: " << toString(AB, AN+BN) << endl;
}

int main (int argc, char *argv[])
{
  	struct timeval startt, endt, result;
	int i, j, k, nt, t, nA, nB, c;
	void *status;
   	pthread_attr_t attr;
  	tThreadArg x[NUM_THREADS];
  	
 	// If instructed, perform a correctness test
  	if (argc > 1 && string(argv[1]) == "correctness") {
  		return presentCorrectness();
  	}
	
  	result.tv_sec = 0;
  	result.tv_usec= 0;

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

   	/* Initialize and set thread detached attribute */
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	printf("|NSize|Iterations|Seq|Th01|Th02|Th04|Th08|Par16|\n");

	// for each input size
	for(c=0; c<NSIZE; c++){
		nA=ANs[c];
		nB=BNs[c];
		printf("| %d-%d | %d |",nA,nB,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(nA, nB);
			seq_function(nA, nB);
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);
		
		// verify correctness
		#ifndef NDEBUG
		bool seqcorrect = true;
		bool parcorrect = true;
		for (int i=1 ; i < nA + nB ; i++)
			if (AB[i-1] >= AB[i])
				seqcorrect = false;
		#endif

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

			result.tv_sec=0; result.tv_usec=0;
			for (j=1; j<=/*NUMTHRDS*/nt; j++)
    		{
				x[j].id = j; 
				x[j].nrT=nt; // number of threads in this round
				x[j].nA=nA;  //input size
				x[j].nB=nB;
				pthread_create(&callThd[j-1], &attr, par_function, (void *)&x[j]);
			}
			
			gettimeofday (&startt, NULL);
			for (t=0; t<TIMES; t++) 
			{
				init(nA, nB);
				// Threads wait for initialisation to complete
				pthread_barrier_wait(&internal_barr);
				// Main thread waits before resetting input
				pthread_barrier_wait(&completed_barr);
			}
			gettimeofday (&endt, NULL);

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
   			result.tv_usec += (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
			printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);
			
			#ifndef NDEBUG
			for (int i=1 ; i < nA + nB ; i++)
				if (AB[i-1] > AB[i])
					parcorrect = false;
			#endif
		}
		printf("\n");
		
		#ifndef NDEBUG
		if (!seqcorrect)
			cerr << "SEQ FAILED!!!" << endl;
		if (!parcorrect)
			cerr << "PAR FAILED!!!" << endl;
		#endif
	}
	pthread_exit(NULL);
}

