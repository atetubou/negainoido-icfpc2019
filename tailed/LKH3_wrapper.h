#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "gflags/gflags.h"

#include "base/ai.h"

typedef std::vector<std::vector<int>> dist_matrix_t;
using pos = std::pair<int, int>;

DECLARE_string(LKH3path);

std::string tempfile_name();

std::vector<int> SolveTSPByLKH3(const dist_matrix_t &d, const char *path_to_LKH3);

std::vector<std::vector<pos>> get_groups(const AI& ai,
					 const std::vector<pos>& tsp_tours);

std::vector<std::pair<int, int>> tikutaOrder(const AI& ai, int n,
					     std::vector<std::pair<int, int>> want_visit=std::vector<std::pair<int, int>>());
