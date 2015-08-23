#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <ctime>

using namespace std;

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
	int logn = log2(n);
	for (int i = 0; i < list.size(); i++)
		list[i]--; // Switch to zero-based
		
	
}
