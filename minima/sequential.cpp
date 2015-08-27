#include <iostream>
#include <sstream>
#include <fstream>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>

using namespace std;

template<class T, size_t N>
size_t size(T(&)[N]) { return N; }

void SequentialMinima(const int data[], int dataCount, int preffix[], int suffix[]) {
	// preffix
	int runningMin = data[0];
	for (int i = 0; i < dataCount; i++) {
		if (data[i] < runningMin)
			runningMin = data[i];
		preffix[i] = runningMin;
	}

	// suffix
	runningMin = data[dataCount - 1];
	for (int i = dataCount - 1; i >= 0; i--) {
		if (data[i] < runningMin)
			runningMin = data[i];
		suffix[i] = runningMin;
	}
}

string arrayToString(const int data[], int dataCount) {
	stringstream ss;
	for (int i = 0; i < dataCount; i++) {
		ss << setfill(' ') << setw(4) << data[i] << ", ";
	}
	return ss.str();
}

vector<int>* readData(string filename) {
	vector<int> *data = new vector<int>();
	ifstream datafile;
	datafile.open(filename.c_str());
	while (!datafile.eof()) {
		int item;
		datafile >> item;
		data->push_back(item);
	}
	datafile.close();
	return data;
}

int main(int argc, char* args[])
{
	if (argc < 2) {
		cerr << "No data file given" << endl;
		return 1;
	}
	
	vector<int> &data = *readData(args[1]);

	//cout << "Starting calculations" << endl;
	//cout << "Data size: " << data.size() << endl;

	int *prefices = new int[data.size()];
	int *suffices = new int[data.size()];

	SequentialMinima(&data[0], data.size(), prefices, suffices);

	//cout << "Data: " << arrayToString(&data[0], data.size()) << endl;
	cout /*<< "Pref: "*/ << arrayToString(prefices, data.size()) << endl;
	cout /*<< "Suff: "*/ << arrayToString(suffices, data.size()) << endl;
	
	delete &data;

	return 0;
}






