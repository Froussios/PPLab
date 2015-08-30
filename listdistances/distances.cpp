#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/time.h>
#include <assert.h>

// Number of threads
#define NUM_THREADS 32

// Number of iterations
#define TIMES 100

// Input Size
#define NSIZE 7
#define NMAX 262144
int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};  

using namespace std;


typedef struct __ThreadArg {
  int id;
  int nrT;
  int n;
} tThreadArg;


pthread_t callThd[NUM_THREADS];
pthread_mutex_t mutexpm;
pthread_barrier_t internal_barr, completed_barr, phase_barr;

// Seed Input
int list[NMAX];

// Subset
int jumplist1[NMAX];
int distance1[NMAX];
int jumplist2[NMAX];
int distance2[NMAX];

// Utility for printing arrays
string toString(int a[], int n, string delim=" ") {
	ostringstream oss;
	for (int i=0 ; i<n ; i++)
		oss << setw(3) << a[i] << delim;
	return oss.str();
}

int log2(int i) {
	int log = 0;
	while (i >>= 1) ++log;
	return log;
}


void init(int n){
	/* Initialize the input for this iteration*/
	for (int i=0 ; i<n ; i++)
		jumplist1[i] = list[i];
}

// Create a random list of length n onto a[]
void createList(int a[], int n) {
	std::vector<int> ints(n);
	for (int i=0 ; i<n ; i++)
		ints[i] = i;
		
	srand ( time(NULL) );
	std::random_shuffle(ints.begin(), ints.end());
	
	for (int i=0 ; i+1<n ; i++)
		a[ints[i]] = ints[i+1];
	a[ints[n-1]] = -1;
	
	#ifndef NDEBUG
	// Verify list
	int cur = ints[0];
	int count = 0;
	while (cur != -1) {
		cur = a[cur];
		count++;
	}
	assert(count == n);
	#endif
}

// Execute algorithm sequentially
void seq_function(int n){
	int logn = log2(n);
	
	int *distance = distance1;
	int *jumplist = jumplist1;
	int *distanceNew = distance2;
	int *jumplistNew = jumplist2;
	
	// initialize distances
	for (int i = 0; i < n; i++) {
		distanceNew[i] = distance[i] = (jumplist[i] == -1) ? 0 : 1;
		jumplistNew[i] = jumplist[i];
	}

	// jump pointers
	for (int y = 0; y < logn ; y++) {
		for (int i = 0; i < n; i++) {
			if (jumplist[i] != -1) {
				distanceNew[i] = distance[i] + distance[jumplist[i]];
				jumplistNew[i] = jumplist[jumplist[i]];
			}
			else {
				distanceNew[i] = distance[i];
				jumplistNew[i] = jumplist[i];
			}
		}

		std::swap(jumplist, jumplistNew);
		std::swap(distance, distanceNew);
	}
	
	// Final result should always be on distance1
	if (distance != distance1) {
		assert(distance == distance2);
		for (int i=0 ; i<n ; i++) {
			distance1[i] = distance2[i];
			
			#ifndef NDEBUG
			// The jumplist will be used to very correctness
			jumplist1[i] = jumplist2[i];
			#endif
		}
	}
}

// Thread function for parallel execution
void* par_function(void* a){	
	tThreadArg *args = (tThreadArg*) a;
	int threadRank = args->id - 1;
	int threadCount = args->nrT;
	int n = args->n;
	int logn = log2(n);
	int i;
	
	int *distance = NULL;
	int *jumplist = NULL;
	int *distanceNew = NULL;
	int *jumplistNew = NULL;
	
	for (int t=0 ; t<TIMES ; t++) {
		// Wait for data to initialised for this iteration
		pthread_barrier_wait(&internal_barr);
		
		distance = distance1;
		jumplist = jumplist1;
		distanceNew = distance2;
		jumplistNew = jumplist2;
		
		// Determine boundaries for this thread
		int first = threadRank * (n / threadCount);
		int last = (threadRank == threadCount-1) ? 
					n : (threadRank + 1) * (n / threadCount);
		
		// Initialize distances and second buffer
		for (int i = first; i < last; i++) {
			distanceNew[i] = distance[i] = (jumplist[i] == -1) ? 0 : 1;
			jumplistNew[i] = jumplist[i];
		}
		
		// Wait for initialisation to complete
		pthread_barrier_wait(&phase_barr);

		// jump pointers
		for (int y = 0; y < logn ; y++) {
			for (int i = first; i < last; i++) {
				if (jumplist[i] != -1) {
					distanceNew[i] = distance[i] + distance[jumplist[i]];
					jumplistNew[i] = jumplist[jumplist[i]];
				}
				else {
					distanceNew[i] = distance[i];
					jumplistNew[i] = jumplist[i];
				}
			}

			std::swap(jumplist, jumplistNew);
			std::swap(distance, distanceNew);
			
			pthread_barrier_wait(&phase_barr);
		}
		
		// Final result should always be on distance1
		if (distance != distance1) {
			assert(distance == distance2);
			for (int i=first ; i<last ; i++) {
				distance1[i] = distance2[i];
				
				#ifndef NDEBUG
				// The jumplist is used for debugging
				jumplist1[i] = jumplist2[i];
				#endif
			}
		}
		
		// This iteration completed
		pthread_barrier_wait(&completed_barr);
	}
	
	pthread_exit(0);
}


// Instead of speedtests, show a readable correctness test
int presentCorrectness() {
	const int N = 16;
	const int nt = 4;
	void *status;
   	pthread_attr_t attr;
  	tThreadArg x[NUM_THREADS];
  	
  	cout << "Input is zero-based" << endl;
  	int indices[N];
  	for (int i=0 ; i<N ; i++)
  		indices[i] = i;
  	cout << "Index: " << toString(indices, N, "|") << endl;
	
	createList(list, N);
	cout << "List:  " << toString(list, N, "|") << endl;
	
	cout << "Sequential:" << endl;
	init(N);
	seq_function(N);
	cout << "Distances: " << toString(distance1, N) << endl;
	
	
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
		// Threads wait for initialisation to complete
		pthread_barrier_wait(&internal_barr);
		// Main thread waits for completion before resetting the input
		pthread_barrier_wait(&completed_barr);
	}

	/* Wait on the other threads */
	for(int j=0; j<nt; j++)
		pthread_join(callThd[j], &status);

	if (pthread_barrier_destroy(&completed_barr) ||
			pthread_barrier_destroy(&internal_barr) ||
			pthread_barrier_destroy(&phase_barr)) {
		printf("Could not destroy the barrier\n");
        return -1;
	}
	
	cout << "Distances: " << toString(distance1, N) << endl;
}


int main (int argc, char *argv[])
{
  	struct timeval startt, endt, result;
	int i, j, k, nt, t, n, c;
	void *status;
   	pthread_attr_t attr;
  	tThreadArg x[NUM_THREADS];
  	
  	// If instructed, perform a correctness test
  	if (argc > 1 && string(argv[1]) == "correctness") {
  		return presentCorrectness();
  	}
	
  	result.tv_sec = 0;
  	result.tv_usec= 0;
  	
   	/* Initialize and set thread detached attribute */
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	printf("|NSize|Iterations|Seq|Th01|Th02|Th04|Th08|Par16|\n");

	// for each input size
	for(c=0; c<NSIZE; c++){
		n=Ns[c];
		
		// Generate a random list on the seed array
		createList(list, n);
		
		printf("| %d | %d |",n,TIMES);

		/* Run sequential algorithm */
		result.tv_usec=0;
		gettimeofday (&startt, NULL);
		for (t=0; t<TIMES; t++) {
			init(n);
			seq_function(n);
		}
		gettimeofday (&endt, NULL);
		
		#ifndef NDEBUG
		// Verify that execution completed correctly
		bool completed = true;
		for (int i = 0; i < n; i++)
			completed = completed && jumplist1[i] == -1;
		if (!completed) {
			std::cerr << "ERROR: SEQ: Jumps not completed" << std::endl;
			std::cerr << print(jumplist1, n) << std::endl;
		}
		#endif
		
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
				// Threads wait for initialisation to complete
				pthread_barrier_wait(&internal_barr);
				// Main thread waits for completion before resetting the input
				pthread_barrier_wait(&completed_barr);
			}
			gettimeofday (&endt, NULL);
			
			#ifndef NDEBUG
			// Verify that execution completed
			bool completed = true;
			for (int i = 0; i < n; i++)
				completed = completed && jumplist1[i] == -1;
			if (!completed) {
				std::cerr << "ERROR: PAR: Jumps not completed" << std::endl;
				std::cerr << print(jumplist1, n) << std::endl;
			}
			#endif

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

