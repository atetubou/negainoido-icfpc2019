/*
 * Wrapper for LKH3
 * http://akira.ruc.dk/~keld/research/LKH-3/
 */


#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <experimental/filesystem>
using namespace std;


typedef vector<vector<int>> dist_matrix_t;

string tempfile_name() {
	char buffer[L_tmpnam + 1];
	char *p = tmpnam(buffer);
	string name(p);
	return name;
}

vector<int> SolveTSPByLKH3(const dist_matrix_t &d, const char *path_to_LKH3) {
	int n = (int)d.size();

	/* Write down distance matrix to a file */
	string file_tsp_name = tempfile_name();

	ofstream file_tsp(file_tsp_name.c_str());
	file_tsp << "NAME: swiss42" << endl;
	file_tsp << "TYPE: TSP"  << endl;
	file_tsp << "COMMENT: this is automatically generated by SolveTSPByLKH3"  << endl;
	file_tsp << "DIMENSION: " << n << endl;
	file_tsp << "EDGE_WEIGHT_TYPE: EXPLICIT"  << endl;
	file_tsp << "EDGE_WEIGHT_FORMAT: FULL_MATRIX "  << endl;
	file_tsp << "EDGE_WEIGHT_SECTION"  << endl;

	for(int i = 0; i < n; i++) {
		assert((int)d[i].size() == n);
		for(int j = 0; j < n; j++) {
			if (j) file_tsp << " ";
			file_tsp << d[i][j];
		}
		file_tsp << endl;
	}

	file_tsp << "EOF"  << endl;

	file_tsp.close();


	/* Write parameter file */
	string file_par_name = tempfile_name();
	string file_output_name = tempfile_name();
	ofstream file_par(file_par_name.c_str());
	file_par << "PROBLEM_FILE = " << file_tsp_name.c_str() << endl;
	file_par << "MOVE_TYPE = 5" << endl;
	file_par << "PATCHING_C = 3" << endl;
	file_par << "PATCHING_A = 2" << endl;
	file_par << "RUNS = 1" << endl;
	file_par << "OUTPUT_TOUR_FILE = " << file_output_name << endl;
	file_par.close();


	string sh = path_to_LKH3;
	sh += " ";
	sh += file_par_name;
	sh += " >&2";
	int res = system(sh.c_str());

	if (res != 0) {
		cerr << "Error while running LKH3" << endl;
		exit(res);
	}


	/* Example output:
NAME : swiss42.3.tour
COMMENT : Length = 3
COMMENT : Found by LKH [Keld Helsgaun] Sat Jun 22 02:08:35 2019
TYPE : TOUR
DIMENSION : 3
TOUR_SECTION
1
2
3
-1
EOF
*/
	ifstream in(file_output_name.c_str());
	string buf;

	while(getline(in, buf)) {
		if (buf == "TOUR_SECTION") {
			break;
		}
	}
	vector<int> tour;
	int v;
	while(in >> v) {
		if (v == -1) break;
		tour.push_back(v - 1);
	}
	in.close();

	return tour;

}

/*
int main() {
	dist_matrix_t v(3, vector<int>(3, 1));
	for(int i=0; i<3; i++) v[i][i] = 0;
	SolveTSPByLKH3(v, "./LKH");
}
*/
