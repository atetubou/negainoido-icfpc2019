#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cassert>


typedef std::vector<std::vector<int>> dist_matrix_t;

std::string tempfile_name();

std::vector<int> SolveTSPByLKH3(const dist_matrix_t &d, const char *path_to_LKH3) {

