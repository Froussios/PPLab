#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <ctime>
#include <assert.h>

using namespace std;

int main(int argc, char *args[]) {
	
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
	
	// Merge into C
	vector<int> C;
	vector<int>::iterator itA = A.begin();
	vector<int>::iterator itB = B.begin();
	while (itA != A.end() && itB != B.end()) {
		if (*itA < *itB) {
			C.push_back(*itA);
			itA++;
		}
		else {
			C.push_back(*itB);
			itB++;
		}
	}
	while (itA != A.end()) {
		C.push_back(*itA);
		itA++;
	}
	while (itB != B.end()) {
		C.push_back(*itB);
		itB++;
	}
	
	// Print
	for (int i=0 ; i<C.size() ; i++)
		cout << C[i] << ' ';
	cout << endl;
}


