// vim-compile: bazel build --verbose_failures //tailed/...
/*

example usage:
$ bazel build //tikuta_solver/...
$ ./bazel-bin/tikuta_solver/tikuta_solver --problem part-1-initial/prob-001.in
WWASSAWWASSAWWASSWAADDWDSSDWWDSSDWWDSS
*/


#include <bits/stdc++.h>
#include <glog/logging.h>
#include <gflags/gflags.h>


#define rep(i, n) for(int i=0; i<int(n); ++i)

DEFINE_string(LKH3path, "","");

#include "LKH3_wrapper.h"

using namespace std;

/*
using board = std::vector<std::string>;

int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
char dir[] = "WSAD";
*/

typedef pair<int, int> node_t;

/*
void dfs(int sx, int sy, board* b, int* cnt) {
	(*b)[sy][sx] = '#';
	--*cnt;
	if (*cnt == 0) {
		std::cout << std::endl;
		exit(0);
	}

	int w = (*b)[0].size();
	int h = b->size();

	for (int i = 0; i < 4; ++i){
		int nx = sx + dx[i];
		int ny = sy + dy[i];

		if (nx < 0 || ny < 0 || w <= nx || h <= ny) continue;

		if ((*b)[ny][nx] == '#') continue;
		std::cout << dir[i];
		dfs(nx, ny, b, cnt);
		std::cout << dir[i^1];
	}
}
*/

int main(int argc, char *argv[]) {
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	google::InstallFailureSignalHandler();



	int h, w;
	std::cin >> h >> w;
	std::vector<std::string> in(h);
	for (int i = 0; i < h; ++i) {
		std::cin >> in[i];
	}

	std::reverse(in.begin(), in.end());

	int sx = 0, sy = 0;


	map<node_t, int> node2id;
	vector<node_t> nodes;

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			if (in[i][j] != '#') {
				node_t v(i, j);
				nodes.push_back(v);
				node2id[v] = (int)nodes.size() - 1;
			}
			if (in[i][j] != 'W') {
				continue;
			}
			sx = j;
			sy = i;
		}
	}

	int n = (int)nodes.size();
	dist_matrix_t dist(n, vector<int>(n, 0));
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			if (i == j) {
				dist[i][j] = 0;
				continue;
			}
			node_t v = nodes[i];
			node_t w = nodes[j];
			if (abs(v.first - w.first) + abs(v.second - w.second) == 1)
				dist[i][j] = 1;
			else
				dist[i][j] = 1<<29;
		}
	}
	rep(k, n) rep(i, n) rep(j, n)
		if (dist[i][j] > dist[i][k] + dist[k][j])
			dist[i][j] = dist[i][k] + dist[k][j];


	vector<int> tour = SolveTSPByLKH3(dist, FLAGS_LKH3path.c_str());

	for(auto i : tour) {
		cout << nodes[i].first << " " << nodes[i].second << endl;
	}

}
