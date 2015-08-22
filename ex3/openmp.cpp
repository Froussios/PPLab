#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <ctime>


using namespace std;


void readData(string fileName, vector<int>& container) {
	ifstream fdata(fileName);
	std::copy(
		std::istream_iterator<int>(fdata),
		std::istream_iterator<int>(),
		std::back_inserter(container));
	fdata.close();
}

template <typename T>
void print(vector<T> v) {
	for (auto i : v)
		cout << setw(2) << i << ", ";
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
	cout << "Start" << endl;

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

	//print(list);

	// Calculate
	int *distance = new int[n];
	int *distanceNew = new int[n];
	int *jumplist = new int[n];
	int *jumplistNew = new int[n];
	{
		// initialize
		for (int i = 0; i < list.size(); i++) {
			distance[i] = distanceNew[i] = (list[i] == -1) ? 0 : 1;
			jumplist[i] = jumplistNew[i] = list[i];
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

			// swap
			swap(jumplist, jumplistNew);
			swap(distance, distanceNew);
		}
	}

#ifndef NDEBUG
	bool completed = true;
	for (int i = 0; i < n; i++)
		completed = completed && jumplist[i] == -1;
	cout << "Jumping compelted: " << ((completed) ? "true" : "false") << endl;
	print(distance, n);
#endif

#ifdef _WIN32
	string dummy;
	getline(cin, dummy);
#endif

    return 0;
}

