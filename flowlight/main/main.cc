#include <ctime>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include "base/graph.h"
#include <glog/logging.h>
using namespace std;

#define REP2(i, m, n) for(int i = (int)(m); i < (int)(n); i++)
#define REPD(i, n) for(int i = (int)(n) - 1; i >= 0; i--)
#define REP(i, n) REP2(i, 0, n)
#define ALL(c) (c).begin(), (c).end()
#define PB(e) push_back(e)
#define FOREACH(i, c) for(auto i = (c).begin(); i != (c).end(); ++i)
#define MP(a, b) make_pair(a, b)
#define BIT(n, m) (((n) >> (m)) & 1)

typedef long long ll;

template <typename S, typename T> ostream &operator<<(ostream &out, const pair<S, T> &p) {
  out << "(" << p.first << ", " << p.second << ")";
  return out;
}

template <typename T> ostream &operator<<(ostream &out, const vector<T> &v) {
  out << "[";
  REP(i, v.size()){
    if (i > 0) out << ", ";
    out << v[i];
  }
  out << "]";
  return out;
}

const int MAX_H = 1000;
const int MAX_W = 1000;
const int MAX_V = MAX_H * MAX_W;

typedef pair<int, int> Position;

int to_index(const Position &position) {
  return position.first * MAX_W + position.second;
}

int to_index(int h, int w) {
  return h * MAX_W + w;
}

Position to_position(const int index) {
  return Position(index / MAX_W, index % MAX_W);
}

class UnionFind{
  int num_components;
  std::vector<int> parent;
  std::vector<int> weight;
  std::vector<int> rank;
public:
  UnionFind(int N) : num_components(N),
                     parent(std::vector<int>(N)),
                     weight(std::vector<int>(N, 1)),
                     rank(std::vector<int>(N, 0)){
    for(int i = 0; i < N; i++) parent[i] = i;
  }
  
  int find(int x){
    if(x == parent[x]) return x;
    else return parent[x] = find(parent[x]);
  }
  
  int size(int x){
    return weight[find(x)];
  }

  
  bool same(int x, int y){
    return find(x) == find(y);
  }
    
  void unite(int x, int y){
    x = find(x);
    y = find(y);
    if(x == y) return;

    num_components--;
    if(rank[x] < rank[y]){
      weight[y] += weight[x];
      parent[x] = y;
    }else{
      weight[x] += weight[y];
      parent[y] = x;
      if(rank[x] == rank[y]) rank[y]++;
    }
  }
  
  int count(){
    return num_components;
  }
};

struct Edge {
  int src_index;
  int dst_index;
  int weight;
  Edge(const int src_index, const int dst_index, int weight) :
    src_index(src_index), dst_index(dst_index), weight(weight) { }

  Edge reverse() const {
    return Edge(dst_index, src_index, weight);
  }

  friend ostream &operator<<(ostream &out, const Edge &e) {
    out << "Edge(" << e.src_index << to_position(e.src_index)  << ", " << e.dst_index << to_position(e.dst_index) << ")";
    return out;
  }
};

vector<Edge> compute_traversal(const int v, const vector<vector<Edge>> &G, vector<int> &visited) {
  visited[v] = 1;
  vector<Edge> es;
  for (const auto &e: G[v]) {
    if (!visited[e.dst_index]) {
      es.push_back(e);
      vector<Edge> sub_es = compute_traversal(e.dst_index, G, visited);
      for (const auto &sub_e: sub_es) {
        es.push_back(sub_e);
      }
      es.push_back(e.reverse());
    }
  }
  return es;
}

vector<Edge> compute_traversal(const int startIndex, const vector<vector<Edge>> &G) {
  vector<int> visited(G.size());
  return compute_traversal(startIndex, G, visited);
}

string get_move(GraphDistance &gd, const int src_index, int dst_index) {
  vector<int> paths;
  int d = gd.shortest_path(src_index, dst_index, paths);
  LOG_IF(FATAL, d < 1 || d + 1 != int(paths.size()));
  // cerr << src_index << " " << dst_index  << " " << d << endl;

  string res;
  for (size_t i = 0; i + 1 < paths.size(); i++) {
    const int dh = to_position(paths[i + 1]).first - to_position(paths[i]).first;
    const int dw = to_position(paths[i + 1]).second - to_position(paths[i]).second;
    if (dh == 1) {
      res += "W";
    } else if (dh == -1) {
      res += "S";
    } else if (dw == 1) {
      res +=  "D";
    } else if (dw == -1) {
      res +=  "A";
    } else {
      LOG_IF(FATAL, true);
    }
  }
  return res;
}

void cover(int index, const vector<string> &board, set<int> &covered_vertices) {
  Position position = to_position(index);
  const int H = board.size();
  const int W = board[0].size();
  const int h = position.first;
  const int w = position.second;
  if (w + 1 < W) {
    if (h > 0 && board[h - 1][w + 1] != '#') {
      covered_vertices.insert(to_index(h - 1, w + 1));
    }
    if (board[h][w + 1] != '#') {
      covered_vertices.insert(to_index(h, w + 1));
    }
    if (h + 1 < H && board[h + 1][w + 1] != '#') {
      covered_vertices.insert(to_index(h + 1, w + 1));
    }
  }
}

bool is_wrapped(int index, const vector<string> &board, const set<int> &wrapped_vertices) {
  Position position = to_position(index);
  const int H = board.size();
  const int W = board[0].size();
  const int h = position.first;
  const int w = position.second;
  if (wrapped_vertices.count(index) == 0) return false;

  if (w + 1 < W) {
    if (h > 0 && board[h - 1][w + 1] != '#') {
      if (wrapped_vertices.count(to_index(h - 1, w + 1)) == 0) return false;
    }
    if (board[h][w + 1] != '#') {
      if (wrapped_vertices.count(to_index(h, w + 1)) == 0) return false;
    }
    if (h + 1 < H && board[h + 1][w + 1] != '#') {
      if (wrapped_vertices.count(to_index(h + 1, w + 1)) == 0) return false;
    }
  }
  return true;
}

int dh[] = {-1, 0, 1, 0};
int dw[] = {0, -1, 0, 1};


int main(int argc, char** argv) {
  int H, W;
  cin >> H >> W;
  vector<string> board(H);

  REP(i, H) cin >> board[H - 1 - i];

  // find start index
  int start_index = -1;
  REP(h, H) REP(w, W) {
    if (board[h][w] == 'W') {
      start_index = to_index(h, w);
    }
  }
  LOG_IF(FATAL, start_index < 0);

  GraphDistance original_gd(MAX_V);
  REP(h, H) REP(w, W) {
    if (board[h][w] == '#') continue;
    REP(k, 4) {
      int nh = h + dh[k];
      int nw = w + dw[k];
      if (0 <= nh && nh < H && 0 <= nw && nw < W && board[nh][nw] != '#') {
        original_gd.add_edge(to_index(h, w), to_index(nh, nw), 1);
      }
    }
  }

  set<int> required_vertices;
  set<int> covered_vertices;
  REP(h, H) REP(w, W - 1) {
    if (h % 3 == 1 && board[h][w] != '#') {
      required_vertices.insert(to_index(h, w));
      covered_vertices.insert(to_index(h, w));
      cover(to_index(h, w), board, covered_vertices);
    }
  }

  REP(h, H) REP(w, W) {
    int index = to_index(h, w);
    if (board[h][w] != '#' && covered_vertices.count(index) == 0) {
      required_vertices.insert(index);
      cover(index, board, covered_vertices);
    }
  }

  vector<Edge> edges;
  for (int v: required_vertices) {
    set<int> neighbors = original_gd.enumerate_neighbors(v, 5);
    for (int w: neighbors) {
      if (required_vertices.count(w)) {
        edges.push_back(Edge(v, w, original_gd.shortest_path(v, w)));
      }
    }
  }
  


  sort(ALL(edges), [](const Edge &e1, const Edge &e2) { return e1.weight < e2.weight; } );
  vector<Edge> mst_edges;
  UnionFind uf(MAX_H * MAX_W);

  for (const auto &e: edges) {
    if (!uf.same(e.src_index, e.dst_index)) {
      mst_edges.push_back(e);
      uf.unite(e.src_index, e.dst_index);
    }
  }
  
  vector<vector<Edge>> G(MAX_V);
  for (const auto &e: mst_edges) {
    G[e.src_index].push_back(e);
    G[e.dst_index].push_back(e.reverse());
  }
  size_t target_count = 0;
  REP(h, H) REP(w, W) if (board[h][w] != '#') target_count++;

  
  string res = "";
  vector<Edge> edge_order = compute_traversal(start_index, G);

  set<int> visited_indices, wrapped_indices;
  visited_indices.insert(start_index);
  cover(start_index, board, wrapped_indices);
  int prev_index = start_index;
  
  for (const auto &e: edge_order) {
    int next_index = e.dst_index;
    if (visited_indices.count(next_index) == 0 && !is_wrapped(next_index, board, wrapped_indices)) {
      const string command = get_move(original_gd, prev_index, next_index);
      int curr_index = prev_index;
      for (char c: command) {
        int h = to_position(curr_index).first;
        int w = to_position(curr_index).second;
        switch(c) {
        case 'W': h++; break;
        case 'S': h--; break;
        case 'A': w--; break;
        case 'D': w++; break;
        default:
          LOG_IF(FATAL, true);
        }
        curr_index = to_index(h, w);
        visited_indices.insert(curr_index);
        cover(curr_index, board, wrapped_indices);
      }
      LOG_IF(FATAL, curr_index != next_index);
      res += command;
      visited_indices.insert(next_index);
      prev_index = next_index;
    }

    if (wrapped_indices.size() == target_count) {
      break;
    }
  }
  cerr << res.size() << endl;
  cout << res << endl;
  return 0;
  
}
