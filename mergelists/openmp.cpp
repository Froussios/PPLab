#include <omp.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iterator>
#include <ctime>
#include <assert.h>

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
int binaryRank(const vector<int>& a, int start, int end, int item) {
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

bool isIncreasing(vector<int> v) {
	for (int i=0 ; i<v.size()-1 ; i++)
		if (v[i] >= v[i+1])
			return false;
	return true;
}

int main(int argc, char *args[])
{
	stopwatch swInput, swParallel, swOutput;

	// Read arguments
	string infile("data.txt"), outfile("output.txt");
	for (int i = 0; i < argc; i++) {
		string arg(args[i]);
		if (arg == "-i")
			infile = args[++i];
		if (arg == "-o")
			outfile = args[++i];
	}

	// Read data
	vector<int> A;
	vector<int> B;
	{
		cout << "Reading " << infile << endl;

		ifstream fdata(infile.c_str());
		string strA, strB;
		getline(fdata, strA);
		getline(fdata, strB);
		fdata.close();
		istringstream issA(strA), issB(strB);
		std::copy(
			std::istream_iterator<int>(issA),
			std::istream_iterator<int>(),
			std::back_inserter(A));
		std::copy(
			std::istream_iterator<int>(issB),
			std::istream_iterator<int>(),
			std::back_inserter(B));

		cout << "Read A[" << A.size() << "] and B[" << B.size() << "]" << endl;
	}

	swInput.stop();
	swParallel.restart();
	
	vector<int> AB(A.size() + B.size());
	int i;

#ifndef NDEBUG
	assert(isIncreasing(A));
	assert(isIncreasing(B));
#endif
	
	cout << "Crunching numbers..." << endl;

	// Do merge
	#pragma omp parallel shared(A, B, AB) private(i)
	{
		// Calculate (A:AB)
		#pragma omp for schedule(static) nowait
		for (i = 0; i < A.size(); i++) {
			int rankAB = i + 1 + binaryRank(B, 0, B.size(), A[i]);
			AB[rankAB - 1] = A[i];
		}

		// Calculate (B:AB)
		#pragma omp for schedule(static) nowait
		for (i = 0; i < B.size(); i++) {
			int rankAB = i + 1 + binaryRank(A, 0, A.size(), B[i]);
			AB[rankAB - 1] = B[i];
		}
	}

	swParallel.stop();
	swOutput.restart();

#ifndef NDEBUG
	assert(isIncreasing(AB));
#endif

	// Write output
	{
		cout << "Writing output to " << outfile << endl;
		ofstream fout(outfile.c_str());
		for (int i = 0; i < AB.size(); i++)
			fout << AB[i] << " ";
		fout << endl;
		fout.close();
	}

	swOutput.stop();

	cout << "Times: " << endl;
	cout << swInput.elapsed() << "ms" << " IO time input: " <<  endl;
	cout << swParallel.elapsed() << "ms" << " Parallel computations: "  << endl;
	cout << swOutput.elapsed() << "ms" << " IO time output: " <<  endl;

#ifdef _WIN32
	string dummy;
	getline(cin, dummy);
#endif
    return 0;
}

