#include <omp.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <assert.h>

using namespace std;



template<class T, size_t N>
size_t size(T(&)[N]) { return N; }

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
		ss << setfill(' ') << setw(4) << data[i] << ", ";
	}
	return ss.str();
}


void openmpMinima(int data[], int n, int prefix[], int suffix[]) {
	int leveln = n;
	int height;
	int expand[n*2];
	int reduce[n*2];	
	int logn = ceil(log2(n));
	
	for (int i=0 ; i<n*2 ; i++)
		expand[i] = reduce[i] = -1;
	
	for (int i=0 ; i<n ; i++)
		expand[index(logn, i)] = data[i];
	
	{
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
			
			for (int i=0 ; i<n ; i++)
				suffix[i] = reduce[index(logn, i)];
		}
	}
	
	//cout << "data: " << arrayToString(data, n) << endl;
	//cout << "pref: " << arrayToString(prefix, n) << endl;
	//cout << "suff: " << arrayToString(suffix, n) << endl;
	
}

main ()
{
	//int data[] =
	//{
	//	58,   89,   32,   73,   131,  156,   30,   29,
	//	141,   37,   133,  151,   88,   53,   122,  126,
	//	131,  49,   130,  115,   16,   83,   40,   145,
	//	10,   112,   20,   147,   14,   104,  111,   92
	//};
	int data[] = {5, 10, 7, 2, 12, 8, 9, 6};
	int dataCount = size(data);

	//cout << "Starting calculations" << endl;
	//cout << "Data size: " << dataCount << endl;

	int *prefices = new int[dataCount];
	int *suffices = new int[dataCount];

	openmpMinima(data, dataCount, prefices, suffices);

	//cout << "Data: " << arrayToString(data, dataCount) << endl;
	//cout << "Pref: " << arrayToString(prefices, dataCount) << endl;
	//cout << "Suff: " << arrayToString(suffices, dataCount) << endl;
	
	//cout << "Check: " << verify(data, dataCount, prefices, suffices) << endl;

	return 0;
}
