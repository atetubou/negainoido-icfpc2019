// vim-compile: bazel build --verbose_failures //tailed/... && ../bazel-bin/tailed/tailed < ../part-1-initial/prob-002.in --LKH3path ./LKH


#include <bits/stdc++.h>
#include <glog/logging.h>
#include <gflags/gflags.h>


#define rep(i, n) for(int i=0; i<int(n); ++i)

DEFINE_string(LKH3path, "","");
const int K = 300; // threshold

#include "LKH3_wrapper.h"

using namespace std;

int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
char dir2chr[] = "WSAD";
char rot2chr[] = "EQ";


/*
using board = std::vector<std::string>;

int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
char dir[] = "WSAD";
*/

typedef pair<int, int> node_t;

node_t add(const node_t &a, const node_t &b) {
	return node_t(a.first + b.first, a.second + b.second);
}

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
set<node_t> unfilled_nodes;
string code; /* answer */

node_t cur_pos;
int cur_direction;
set<node_t> manipulator;


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

node_t rotate(node_t v, int d) {
	d = (d % 4 + 4) % 4;
	rep(i, d) {
		node_t w;
		w.first = -v.second;
		w.second = v.first;
		v = w;
	}
	return v;
}

class state_t{
public:
	node_t pos;
	int dir;

	bool operator <(const state_t &a) const {
		return make_pair(pos, dir) < make_pair(a.pos, a.dir);
	}

	state_t() : dir(0) {}
	state_t(node_t pos, int dir) : pos(pos), dir(dir) {}
};

void do_operation(char op) {
//	cerr << op << endl;
	code += op;
	string::size_type d = string(dir2chr).find(op);
	if (d == string::npos) {
		if (op == 'Q') {
			cur_direction = (cur_direction + 1) % 4;
		} else if (op == 'E') {
			cur_direction = (cur_direction - 1 + 4) % 4;
		} else {
			LOG(FATAL) << "Unexpected operation" << endl;
		}
	} else {
		cur_pos.first += dx[d];
		cur_pos.second += dy[d];
	}

	for(auto d : manipulator) {
		node_t v = add(cur_pos, rotate(d, cur_direction));
		unfilled_nodes.erase(v);
	}
}

/* fill target */
void fill_for(const node_t &target) {
	for(auto d : manipulator) {
		node_t v = add(cur_pos, rotate(d, cur_direction));
		unfilled_nodes.erase(v);
	}
	if (!unfilled_nodes.count(target)) return;

	queue<state_t> q;
	map<state_t, char> lastop;
	map<state_t, state_t> parent;
	state_t start_st(cur_pos, cur_direction);
	q.push(start_st);
	lastop[start_st] = 0;
	parent[start_st] = start_st;
	state_t last = start_st;
	bool done = false;
	while(!q.empty()) {
		state_t st = q.front();
		q.pop();

//		cerr << st.pos.first << " " << st.pos.second << " " << st.dir << endl;
		for(auto d : manipulator) {
			node_t v = add(st.pos, rotate(d, st.dir));
			if (v == target) {
//				cerr << v.first << " " << v.second << " " << st.dir << endl;
//				cerr << target.first << " " << target.second << endl;
				last = st;
				done = true;
				break;
			}
		}
		if (done) {
			break;
		}

		/* move */
		rep(d, 4) {
			node_t w(st.pos.first + dx[d], st.pos.second + dy[d]);
			state_t next_st(w, st.dir);
			if (node2id.count(w) && !parent.count(next_st)) {
				lastop[next_st] = dir2chr[d];
				parent[next_st] = st;
				q.push(next_st);
			}
		}

		/* rotate */
		rep(r, 2) {
			state_t next_st(st.pos, (st.dir + 2*r-1 + 4) % 4);
			if (!parent.count(next_st)) {
				lastop[next_st] = rot2chr[r];
				parent[next_st] = st;
				q.push(next_st);
			}
		}

	}
	if (!done) {
		LOG(FATAL) << "Cannot find a way" << endl;
	}

	vector<char> operations;
	state_t v = last;
//	cerr << v.pos.first << " " << v.pos.second << " " << v.dir << " " << lastop[v] << endl;
	while(lastop[v] != 0) {
//		cerr << v.pos.first << " " << v.pos.second << " " << v.dir << " " << lastop[v] << endl;
		operations.push_back(lastop[v]);
		v = parent[v];
	}
	reverse(operations.begin(), operations.end());

	for(auto op : operations) {
		do_operation(op);
	}
//	cerr << "FILLED " << target.first << " " << target.second << endl;
}

void initialize_manipulator() {
	manipulator.insert(node_t(1, 0));
	manipulator.insert(node_t(1, 1));
	manipulator.insert(node_t(1, -1));
	manipulator.insert(node_t(0, 0));
}

vector<int> ExactTSP(const dist_matrix_t &dist) {
	int n = (int)dist.size();
	vector<int> perm(n);
	rep(i, n-1) perm[i] = i+1;
	vector<int> best;
	int bestcost = 1<<29;
	do {
		int cost = 0;
		vector<int> tour;
		tour.insert(tour.begin(), 0);
		rep(i, (int)tour.size()-1)
			cost += dist[tour[i]][tour[i+1]];
		if (bestcost > cost) {
			bestcost = cost;
			best = tour;
		}
	} while(next_permutation(perm.begin(), perm.end()));
	return best;
}

vector<int> solve_path_TSP_on_submatrix(const vector<int> &nodeids, const dist_matrix_t &dist) {
	int T = (int)nodeids.size();
	if (T <= 1) return nodeids;
	dist_matrix_t sub_dist(T, vector<int>(T, 0));
	rep(i, T) rep(j, T) {
		sub_dist[i][j] = dist[nodeids[i]][nodeids[j]];
	}

	vector<int> tour;
	if (T <= 7) {
		tour = ExactTSP(sub_dist);
	} else {
		tour = SolveTSPByLKH3(sub_dist, FLAGS_LKH3path.c_str());
	}
	if (tour[0] != 0) {
		LOG(FATAL) << "I thought that the first point of tour is 0" << endl;
	}
	vector<int> res(tour.size());
	rep(i, tour.size()) {
		res[i] = nodeids[tour[i]];
	}
	return res;
}

int main(int argc, char *argv[]) {
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	google::InstallFailureSignalHandler();

	if (FLAGS_LKH3path == "") {
		FLAGS_LKH3path = "./tailed/LKH";
//		LOG(FATAL) << "Please specify LKH path" << endl;
	}


	int h, w;
	std::cin >> h >> w;
	std::vector<std::string> in(h);
	for (int i = 0; i < h; ++i) {
		std::cin >> in[i];
	}

	std::reverse(in.begin(), in.end());


	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			node_t v(x, y);
			if (in[y][x] == 'W') {
				cur_pos = v;
				continue; // do not push for now; the first index will be start node
			}
			if (in[y][x] != '#') {
				nodes.push_back(v);
				unfilled_nodes.insert(v);
			}
		}
	}
	mt19937 get_rand_mt;
	shuffle(nodes.begin(), nodes.end(), get_rand_mt);
	nodes.insert(nodes.begin(), cur_pos); /* the first node is the start */
	rep(i, nodes.size()) {
		node2id[nodes[i]] = i;
	}

	initialize_manipulator();

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

	int T = min(n, K); /* Number of terminals */
	dist_matrix_t sub_dist(T, vector<int>(T, 0));
	rep(i, T) rep(j, T) {
		sub_dist[i][j] = dist[i][j];
	}

	vector<int> tour = SolveTSPByLKH3(sub_dist, FLAGS_LKH3path.c_str());
	if (tour[0] != 0) {
		LOG(FATAL) << "I thought that the first point of tour is 0" << endl;
	}

	vector<vector<int>> clusters(T);
	rep(i, n) {
		int best = 1<<29;
		int bestid = -1;
		rep(t, T) {
			if (best > dist[i][t]) {
				best = dist[i][t];
				bestid = t;
			}
		}
		clusters[bestid].push_back(i);
	}

	for(int t = 0; t < (int)tour.size(); t++) {
		vector<int> clst = clusters[tour[t]];
		while(t+1 < (int)tour.size() && clst.size() + clusters[tour[t+1]].size() < K) {
			clst.insert(clst.end(), clusters[tour[t+1]].begin(), clusters[tour[t+1]].end());
			t++;
		}
		vector<int> subtour = solve_path_TSP_on_submatrix(clst, dist);

		for(auto i : subtour) {
//			cerr << nodes[i].first << " " << nodes[i].second << endl;
			fill_for(nodes[i]);
		}
	}

	cerr << "solution size: " << code.size() << endl;
	cerr << "    nodes: " << n << endl;
	cout << code << endl;

}
