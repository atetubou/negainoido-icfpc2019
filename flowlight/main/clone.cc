#include <ctime>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include "base/graph.h"
#include "union_find.h"
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


int move_index(int index, char c) {
  int h = to_position(index).first;
  int w = to_position(index).second;
  switch(c) {
  case 'W': h++; break;
  case 'S': h--; break;
  case 'A': w--; break;
  case 'D': w++; break;
  default:
    LOG_IF(FATAL, true);
  }
  return to_index(h, w);
}

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

set<int> collect_required_vertices(const vector<string> &board) {
  const int H = board.size();
  const int W = board[0].size();
  set<int> covered_vertices;
  set<int> required_vertices;
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
  return required_vertices;
}

vector<pair<int, int>> collect_clone_greedy(int start_index, GraphDistance &gd, const vector<int> &clone_vertices) {
  vector<pair<int, int>> res;
  set<int> used;
  
  while (used.size() < clone_vertices.size()) {
    int best_v = -1;
    int best_d = 1e9;
    for (int v : clone_vertices) {
      if (used.count(v)) continue;
      int spd = gd.shortest_path(start_index, v);
      if (spd < best_d) {
        best_d = spd;
        best_v = v;
      }
    }
    used.insert(best_v);
    res.push_back(make_pair(start_index, best_v));
    start_index = best_v;
  }
  return res;
}

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

vector<Edge> compute_traversal(const int start_index, const vector<vector<Edge>> &G) {
  vector<int> visited(G.size());
  return compute_traversal(start_index, G, visited);
}

vector<int> compute_tour(const int start_index, const set<int> &visit_vertices, GraphDistance &gd) {
  vector<Edge> edges;
  for (int v: visit_vertices) {
    set<int> neighbors = gd.enumerate_neighbors(v, 5);
    for (int w: neighbors) {
      if (visit_vertices.count(w)) {
        edges.push_back(Edge(v, w, gd.shortest_path(v, w)));
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
  cerr << "MST-size: " << mst_edges.size() << endl;
  
  vector<vector<Edge>> G(MAX_V);
  for (const auto &e: mst_edges) {
    G[e.src_index].push_back(e);
    G[e.dst_index].push_back(e.reverse());
  }

  vector<Edge> edge_order = compute_traversal(start_index, G);
  // cerr << edge_order.size() << endl;

  vector<int> res;
  res.push_back(start_index);
  for (const auto &e: edge_order) {
    res.push_back(e.dst_index);
  }
  // cerr << res.size() << endl;
  return res;
}

string move(int src_index, int dst_index, set<int> &visited_vertices, set<int> &wrapped_vertices, const vector<string> &board, GraphDistance &gd) {
  const string command = get_move(gd, src_index, dst_index);
  int curr_index = src_index;
  for (char c: command) {
    curr_index = move_index(curr_index, c);
    visited_vertices.insert(curr_index);
    cover(curr_index, board, wrapped_vertices);
  }
  LOG_IF(FATAL, curr_index != dst_index);
  return command;
}

string enjoy_tour(int start_index, const vector<int> &tour, set<int> &visited_indices, set<int> &wrapped_indices,const vector<string> &board,  GraphDistance &original_gd) {
  string res;
  int prev_index = start_index;
  size_t initial_i = start_index != tour[0] ? 0 : 1;

  for (size_t i = initial_i; i < tour.size(); i++) {
    int next_index = tour[i];
    if (visited_indices.count(next_index) == 0 && !is_wrapped(next_index, board, wrapped_indices)) {
      const string command = get_move(original_gd, prev_index, next_index);
      int curr_index = prev_index;
      for (char c: command) {
        curr_index = move_index(curr_index, c);
        visited_indices.insert(curr_index);
        cover(curr_index, board, wrapped_indices);
      }
      LOG_IF(FATAL, curr_index != next_index);
      res += command;
      visited_indices.insert(next_index);
      prev_index = next_index;
    }
  }
  return res;
}


int main(int argc, char** argv) {
  int H, W;
  cin >> H >> W;
  vector<string> board(H);

  REP(i, H) cin >> board[H - 1 - i];

  // find start index
  int start_index = -1;
  vector<int> c_vertices;
  vector<int> x_vertices;
  REP(h, H) REP(w, W) {
    if (board[h][w] == 'W') {
      start_index = to_index(h, w);
    }

    if (board[h][w] == 'C') {
      c_vertices.push_back(to_index(h, w));
    }

    if (board[h][w] == 'X') {
      x_vertices.push_back(to_index(h, w));
    }
  }
  LOG_IF(FATAL, start_index < 0);
  

  GraphDistance original_gd(MAX_V);
  int dh[] = {-1, 0, 1, 0};
  int dw[] = {0, -1, 0, 1};

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

  string res = "";
  set<int> visited_indices;
  set<int> wrapped_indices;
  visited_indices.insert(start_index);
  cover(start_index, board, wrapped_indices);

  vector<pair<int, int>> clone_collect_moves = collect_clone_greedy(start_index, original_gd, c_vertices);
  if (!clone_collect_moves.empty() && !x_vertices.empty()) {
    for (const auto &e: clone_collect_moves) {
      int dst_index = e.second;
      res += move(start_index, dst_index, visited_indices, wrapped_indices, board, original_gd);
      start_index = dst_index;
    }

    int best_d = 1e9;
    int best_x = -1;
    for (int v: x_vertices) {
      int spd = original_gd.shortest_path(start_index, v);
      if (best_d > spd) {
        best_d = spd;
        best_x = v;
      }
    }

    res += move(start_index, best_x, visited_indices, wrapped_indices, board, original_gd);
    start_index = best_x;
    cerr << "++++++ LET'S CLONE (" << c_vertices.size() <<  ")++++++" << " " << res << endl;
  }

  
  set<int> required_vertices = collect_required_vertices(board);
  required_vertices.insert(start_index);

  vector<int> tour = compute_tour(start_index, required_vertices, original_gd);
  // cerr << tour.size() << " " << required_vertices.size() << endl;
  if (clone_collect_moves.empty() || x_vertices.empty() || tour.size() == 1) {
    res += enjoy_tour(start_index, tour, visited_indices, wrapped_indices, board, original_gd);
  } else {
    vector<vector<int>> subtours;
    int clone_count = min<int>(1 + c_vertices.size(), tour.size());
    int subtour_size = (tour.size() + clone_count - 1) / clone_count;
    for (size_t i = 0; i < tour.size(); i += subtour_size) {
      size_t start_index = i;
      size_t end_index = min<int>(i + subtour_size, tour.size());
      subtours.push_back(vector<int>(tour.begin() + start_index, tour.begin() + end_index));
    }
    REP(i, clone_count - 1) {
      res += 'C';
    }

    for (size_t i = 0; i < subtours.size(); i++) {
      if (i > 0) {
        res += "#";  
      }
      res += enjoy_tour(start_index, subtours[i], visited_indices, wrapped_indices, board, original_gd);
    }
  }
  cerr << res.size() << endl;
  cout << res << endl;
  return 0;
}
