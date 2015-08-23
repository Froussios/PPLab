#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <ctime>


using namespace std;

class stopwatch {
	unsigned int start, end;
public:
	stopwatch() { restart(); }
	void restart() { start = clock(); }
	void stop() { end = clock(); }
	unsigned int elapsed() { return end - start; }
};


void readData(string fileName, vector<int>& container) {
	ifstream fdata(fileName.c_str());
	std::copy(
		std::istream_iterator<int>(fdata),
		std::istream_iterator<int>(),
		std::back_inserter(container));
	fdata.close();
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
	cout << "Started" << endl;
	stopwatch swRead, swParallel, swWrite;

	// Read arguments
	string inFilename("data.txt"), outFilename("output.txt");
	for (int i = 0; i < argc; i++) {
		string arg(args[i]);
		if (arg == "-i")
			inFilename = args[++i];
		if (arg == "-o")
			outFilename = args[++i];
	}

	// Read data
	vector<int> list;
	readData(inFilename, list);
	int n = list.size();
	int logn = log2(n);
	for (int i = 0; i < list.size(); i++)
		list[i]--; // Switch to zero-based

	// Calculate
	int *distance = new int[n];
	int *distanceNew = new int[n];
	int *jumplist = new int[n];
	int *jumplistNew = new int[n];
	int i;

	swRead.stop();
	swParallel.restart();

	#pragma omp parallel private(i) // pointers are private, data is isn't
	{
		#pragma omp for schedule(static) nowait
		// initialize
		for (i = 0; i < n; i++) {
			distance[i] = distanceNew[i] = (list[i] == -1) ? 0 : 1;
			jumplist[i] = jumplistNew[i] = list[i];
		}

		// jump pointers
		for (int y = 0; y < logn ; y++) {
			#pragma omp for schedule(static) nowait
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

			#pragma omp master
			{
				// swap
				swap(jumplist, jumplistNew);
				swap(distance, distanceNew);
			}
		}
	}

	swParallel.stop();
	swWrite.restart();

	// Write result to file
	fstream fout(outFilename.c_str(), fstream::out);
	for (int i = 0; i < n; i++)
		fout << distance[i] << " ";
	fout << endl;
	fout.close();

	swWrite.stop();

#ifndef NDEBUG
	bool completed = true;
	for (int i = 0; i < n; i++)
		completed = completed && jumplist[i] == -1;
	if (!completed)
		cout << "ERROR: Jumps not completed" << endl;
	//print(jumplist, n);
	print(distance, n);
#endif

	cout << "Completed" << endl;
	cout << swRead.elapsed() << "ms read" << endl;
	cout << swParallel.elapsed() << "ms parallel" << endl;
	cout << swWrite.elapsed() << "ms write" << endl;

#ifdef _WIN32
	string dummy;
	getline(cin, dummy);
#endif

    return 0;
}
