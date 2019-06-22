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

std::vector<int> SolveTSPByLKH3(const dist_matrix_t &d, const char *path_to_LKH3);

// Return tour vising n randomly selected position via LKH3.
std::vector<std::pair<int, int>>
SolveShrinkedTSP(const std::vector<std::string>& board, int n, const std::string& path_to_LKH3);
