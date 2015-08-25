#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 1000

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
pthread_barrier_t barr, internal_barr;

// Seed Input
// A sorted array
int S[NMAX];

// Subset
int A[NMAX];
int B[NMAX];
int sizeA;
int sizeB;

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

// Initialize subset of size n
void init(int n){
	int ia=0, ib=0;
	for (int i=0 ; i<n ; i++)
		if (rand() % 2)
			A[ia++] = S[i];
		else
			B[ib++] = S[i];
	sizeA = ia;
	sizeB = ib;
}

void seq_function(){
	// Do merge
	{
		// Calculate (A:AB)
		for (i = 0; i < sizeA; i++) {
			A_AB[i] = i + 1 + binaryRank(B, 0, sizeB, A[i]);
		}

		// Calculate (B:AB)
		for (i = 0; i < sizeB; i++) {
			B_AB[i] = i + 1 + binaryRank(A, 0, sizeA, B[i]);
		}

		// Merge
		for (i = 0; i < sizeA; i++)
			AB[A_AB[i] - 1] = A[i];
		for (i = 0; i < sizeB; i++)
			AB[B_AB[i] - 1] = B[i];
	}
}

void* par_function(void* a){
	/* The code for threaded computation */
	// Perform operations on B
}

int main (int argc, char *argv[])
{
  	struct timeval startt, endt, result;
	int i, j, k, nt, t, n, c;
	void *status;
   	pthread_attr_t attr;
  	tThreadArg x[NUM_THREADS];
	
  	result.tv_sec = 0;
  	result.tv_usec= 0;

	/* Generate a seed input */
	srand ( time(NULL) );
	s[0] = rand() % 5;
	for(k=1; k<NMAX; k++){
		s[k] = s[k-1] + 1 + (rand() % 5); // Seed will increasing
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
			seq_function();
		}
		gettimeofday (&endt, NULL);
		result.tv_usec = (endt.tv_sec*1000000+endt.tv_usec) - (startt.tv_sec*1000000+startt.tv_usec);
		printf(" %ld.%06ld | ", result.tv_usec/1000000, result.tv_usec%1000000);

		/* Run threaded algorithm(s) */
		for(nt=1; nt<NUM_THREADS; nt=nt<<1){
		        if(pthread_barrier_init(&barr, NULL, nt+1))
    			{
        			printf("Could not create a barrier\n");
			        return -1;
			}
		        if(pthread_barrier_init(&internal_barr, NULL, nt))
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
				pthread_barrier_wait(&barr);
			}
			gettimeofday (&endt, NULL);

			/* Wait on the other threads */
			for(j=0; j</*NUMTHRDS*/nt; j++)
			{
				pthread_join(callThd[j], &status);
			}

			if (pthread_barrier_destroy(&barr)) {
        			printf("Could not destroy the barrier\n");
			        return -1;
			}
			if (pthread_barrier_destroy(&internal_barr)) {
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

