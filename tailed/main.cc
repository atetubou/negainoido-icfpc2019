// vim-compile: bazel build --verbose_failures //tailed/... && ../bazel-bin/tailed/tailed < ../part-1-initial/prob-001.in --LKH3path ./LKH


#include <bits/stdc++.h>
#include <glog/logging.h>
#include <gflags/gflags.h>


#define rep(i, n) for(int i=0; i<int(n); ++i)

DEFINE_string(LKH3path, "","");

#include "LKH3_wrapper.h"

using namespace std;

int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
char dir2chr[] = "WSAD";


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


map<node_t, int> node2id;
vector<node_t> nodes;

string move_from_AtoB(const node_t &s, const node_t &t) {
	queue<pair<int, int>> q;
	map<node_t, int> lastd;
	map<node_t, node_t> parent;
	q.push(s);
	lastd[s] = -1;
	while(!q.empty()) {
		node_t v = q.front();
		q.pop();
		if (v == t) break;
		rep(d, 4) {
			node_t w(v.first + dx[d], v.second + dy[d]);
			if (node2id.count(w) && !lastd.count(w)) {
				lastd[w] = d;
				parent[w] = v;
				q.push(w);
			}
		}
	}
	assert(lastd.count(t));

	vector<int> directions;
	node_t v = t;
	while(lastd[v] != -1) {
		directions.push_back(lastd[v]);
		v = parent[v];
	}
	reverse(directions.begin(), directions.end());

	string seq;
	for(auto d : directions) {
		seq += dir2chr[d];
	}
	return seq;
}

int main(int argc, char *argv[]) {
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	google::InstallFailureSignalHandler();

	if (FLAGS_LKH3path == "") {
		LOG(FATAL) << "Please specify LKH path" << endl;
	}


	int h, w;
	std::cin >> h >> w;
	std::vector<std::string> in(h);
	for (int i = 0; i < h; ++i) {
		std::cin >> in[i];
	}

	std::reverse(in.begin(), in.end());

	node_t start;

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (in[y][x] != '#') {
				node_t v(x, y);
				nodes.push_back(v);
				node2id[v] = (int)nodes.size() - 1;
			}
			if (in[y][x] != 'W') {
				continue;
			}
			start = node_t(x, y);
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

	rep(i, tour.size()) {
		if (tour[i] == node2id[start]) {
			vector<int> tmp;
			tmp.insert(tmp.end(), tour.begin() + i, tour.end());
			tmp.insert(tmp.end(), tour.begin(), tour.begin() + i);
			tour = tmp;
			break;
		}
	}

	string code;
	for(int i = 0; i < (int)tour.size() - 1; i++) {
		code += move_from_AtoB(nodes[i], nodes[i+1]);

		cerr << nodes[i].first << " " << nodes[i].second << endl;
		cerr << move_from_AtoB(nodes[i], nodes[i+1]) << endl;
	}

	cout << code << endl;

}
