#include <omp.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <ctime>
#include <assert.h>

using namespace std;

// Test repetitions
#define TIMES 100

// Input Size
#define NSIZE 7
#define NMAX 262144
int Ns[NSIZE] = {4096, 8192, 16384, 32768, 65536, 131072, 262144};

// Thread configurations
#define MAXTHREADS 8

int list[NMAX];
int distance1[NMAX];
int distance2[NMAX];
int jumplist1[NMAX];
int jumplist2[NMAX];


class stopwatch {
	unsigned int start, end;
public:
	stopwatch() { restart(); }
	void restart() { start = clock(); }
	void stop() { end = clock(); }
	unsigned int elapsed() { return end - start; }
};

// Creates a list the array
// Values are the index of the next element
// -1 denotes no successor
void createList(int a[], int n) {
	std::vector<int> ints(n);
	for (int i=0 ; i<n ; i++)
		ints[i] = i;
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

template <typename T>
void print(vector<T> v) {
	for (int i=0 ; i<v.size() ; i++)
		cout << setw(2) << v[i] << ", ";
	cout << endl;
}

template <typename T>
void print(T *a, int n) {
	for (int i = 0; i < n; i++)
		cout << setw(2) << a[i] << "  ";
	cout << endl;
}

int log2(int i) {
	int log = 0;
	while (i >>= 1) ++log;
	return log;
}


int main(int argc, char *args[])
{
	stopwatch sw;
	int nthreads;
	int i;
	
	cout << "N | # | 1 threads | 2 threads | 4 threads | 8 threads " << endl;
	
	for (int c=0 ; c<NSIZE ; c++) {	
		int n = Ns[c];
		
		// Generate random input of size n
		int *list = new int[n];
		createList(list, n);
		
		cout << n << " | " << TIMES << " | ";
		int *distance = distance1;
		int *jumplist = jumplist1;
		int *distanceNew = distance2;
		int *jumplistNew = jumplist2;
		
		for (int nthreads=1 ; nthreads<=MAXTHREADS ; nthreads <<= 1) {
			
			sw.restart();
			
			// Calculate
			#pragma omp parallel num_threads(nthreads) shared(distance, distanceNew, jumplist, jumplistNew) private(i)
			{
				// Repeat test
				for (int t=0 ; t<TIMES ; t++) {
					int logn = log2(n);
											
					#pragma omp for schedule(static)
					// initialize
					for (i = 0; i < n; i++) {
						distance[i] = distanceNew[i] = (list[i] == -1) ? 0 : 1;
						jumplist[i] = jumplistNew[i] = list[i];
					}

					// jump pointers
					for (int y = 0; y < logn ; y++) {
						#pragma omp for schedule(static)
						for (i = 0; i < n; i++) {
							if (jumplist[i] != -1) {
								distanceNew[i] = distance[i] + distance[jumplist[i]];
								jumplistNew[i] = jumplist[jumplist[i]];
							}
							else {
								distanceNew[i] = distance[i];
								jumplistNew[i] = jumplist[i];
							}
						}
		
						#pragma omp single
						{
							swap(jumplist, jumplistNew);
							swap(distance, distanceNew);
						}
					}
				}	
			}
			
			sw.stop();
		
			cout << sw.elapsed() << " | ";	
		}
		
		cout << endl;
		
		#ifndef NDEBUG
		// Verify that the execution completed
		bool completed = true;
		for (int i = 0; i < n; i++)
			completed = completed && jumplist[i] == -1;
		if (!completed)
			cout << "ERROR: Jumps not completed" << endl;
		#endif
		
		
	}

#ifdef _WIN32
	string dummy;
	getline(cin, dummy);
#endif

    return 0;
}

