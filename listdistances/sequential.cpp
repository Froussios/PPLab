#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <ctime>
#include <assert.h>

using namespace std;

void readData(string fileName, vector<int>& container) {
	ifstream fdata(fileName.c_str());
	std::copy(
		std::istream_iterator<int>(fdata),
		std::istream_iterator<int>(),
		std::back_inserter(container));
	fdata.close();
}

int main(int argc, char *args[]) {
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
	for (int i = 0; i < list.size(); i++)
		list[i]--; // Switch to zero-based
	
	// Find the start of the list
	vector<bool> isstart(n, true);
	for (int i=0 ; i<n ; i++)
		if (list[i] != -1)
			isstart[list[i]] = false;
	int next = -1;
	for (int i=0 ; i<n ; i++)
		if (isstart[i])
			next = i;
			
	assert(next != -1);
	
	// Traverse the list
	// Each node is closer to the end
	int distance = n-1;
	vector<int> distances(n);
	while (next != -1) {
		distances[next] = distance--;
		next = list[next];
	}
	
	// Print result
	ofstream outfile(outFilename.c_str());
	for (int i=0 ; i<n ; i++)
		outfile << distances[i] << ' ';
	outfile << endl;
	outfile.close();
	
}
