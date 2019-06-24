#include <ctime>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include "base/graph.h"
#include "union_find.h"
#include "base/ai.h"
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

template <typename T> ostream &operator<<(ostream &out, const set<T> &v) {
  out << "{";
  int i = 0;
  for (const auto &e: v) {
    if (i > 0) out << ", ";
    out << e;
    i++;
  }
  out << "}";
  return out;
}

const int MAX_H = 1000;
const int MAX_W = 1000;
const int MAX_V = MAX_H * MAX_W;

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
  case 'S': h++; break;
  case 'W': h--; break;
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

  string res;
  for (size_t i = 0; i + 1 < paths.size(); i++) {
    const int dh = to_position(paths[i + 1]).first - to_position(paths[i]).first;
    const int dw = to_position(paths[i + 1]).second - to_position(paths[i]).second;
    if (dh == 1) {
      res += "S";
    } else if (dh == -1) {
      res += "W";
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

void cover(int index, AI &ai, set<int> &covered_vertices) {
  Position position = to_position(index);
  for (auto p: ai.get_relative_manipulator_positions()) {
    Position q = position;
    q.first += p.first;
    q.second += p.second;
    if (ai.reachable(position, q)) {
      covered_vertices.insert(to_index(q.first, q.second));
    }
  }
}

bool is_wrapped(int index, AI &ai, const set<int> &wrapped_vertices) {
  Position position = to_position(index);
  for (auto p: ai.get_relative_manipulator_positions()) {
    Position q = position;
    q.first += p.first;
    q.second += p.second;
    if (ai.reachable(position, q)) {
      if (wrapped_vertices.count(to_index(q)) == 0) return false;
    }
  }
  return true;
}

vector<pair<int, int>> collect_item_greedy(int start_index, GraphDistance &gd, const vector<int> &item_vertices) {
  vector<pair<int, int>> res;
  set<int> used;

  while (used.size() < item_vertices.size()) {
    int best_v = -1;
    int best_d = 1e9;
    for (int v : item_vertices) {
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

vector<int> compute_traversal(const int v, const vector<vector<Edge>> &G, vector<int> &visited) {
  vector<int> vs;
  vs.push_back(v);
  visited[v] = 1;
  vector<vector<int> > sub_vs_list;
  for (const auto &e: G[v]) {
    if (!visited[e.dst_index]) {
      vector<int> sub_vs = compute_traversal(e.dst_index, G, visited);
      sub_vs_list.push_back(sub_vs);
    }
  }

  sort(sub_vs_list.begin(), sub_vs_list.end(), [](const vector<int> &vs1, const vector<int> &vs2) {
      return vs1.size() < vs2.size();
    });

  for (const vector<int> &sub_vs: sub_vs_list) {
    for (int v: sub_vs) {
      vs.push_back(v);
    }
  }
  return vs;
}

vector<int> compute_traversal(const int start_index, const vector<vector<Edge>> &G) {
  vector<int> visited(G.size());
  return compute_traversal(start_index, G, visited);
}

vector<int> compute_tour(const int start_index, AI &ai, GraphDistance &gd) {
  vector<int> tour;
  set<int> filled_vertices;
  const int H = ai.board.size();
  const int W = ai.board[0].size();
  vector<string> debug_board = ai.board;
  REP(h, H) REP(w, W) {
    if (ai.filled[h][w]) {
      filled_vertices.insert(to_index(h, w));
      debug_board[h][w] = 'V';
    }
  }
  // for (auto s: debug_board) {
  //   cerr << s << endl;
  // }
  cerr << "#filled_vertiecs: " << filled_vertices.size() << endl;
  
  vector<Position> relative_positions = ai.get_relative_manipulator_positions();
  
  auto has_unfilled_neighbors = [&ai, &relative_positions, &filled_vertices](int index) {
    int h = to_position(index).first;
    int w = to_position(index).second;
    for (auto p: relative_positions) {
      int nh = h + p.first;
      int nw = w + p.second;
      if (!ai.reachable(Position(h, w), Position(nh, nw))) {
        continue;
      }
      
      if (filled_vertices.count(to_index(nh, nw)) == 0) {
        return true;
      }
    }
    return false;
  };

  auto count_unfilled_neighbors = [&ai, &relative_positions, &filled_vertices](int index) {
    int h = to_position(index).first;
    int w = to_position(index).second;
    int res = 0;
    for (auto p: relative_positions) {
      int nh = h + p.first;
      int nw = w + p.second;
      if (!ai.reachable(Position(h, w), Position(nh, nw))) {
        continue;
      }
      
      if (filled_vertices.count(to_index(nh, nw)) == 0) {
        res++;
      }
    }
    return res;
  };

  int curr_index = start_index;
  while (true) {
    auto dist_and_closest_vertices = gd.find_closest_vertices(curr_index, has_unfilled_neighbors);
    if (dist_and_closest_vertices.first == -1) {
      break;
    }
  
    // cerr << dist_and_closest_vertices << endl;
    int best_next = -1;
    pair<int, int> best_score(0, 0);
    for (int v : dist_and_closest_vertices.second) {
      int nc = count_unfilled_neighbors(v);
      pair<int, int> score(nc, -to_position(v).second);
      if (score > best_score) {
        best_next = v;
        best_score = score;
      }
    }
    LOG_IF(FATAL, best_next < 0);

    vector<int> path;
    gd.shortest_path(curr_index, best_next, path);
    for (size_t i = 1; i < path.size(); i++) {
      tour.push_back(path[i]);
      for (auto p: relative_positions) {
        int nh = to_position(path[i]).first + p.first;
        int nw = to_position(path[i]).second + p.second;
        if (ai.reachable(to_position(path[i]), Position(nh, nw))) {
          filled_vertices.insert(to_index(nh, nw));
        }
      }
    }
    curr_index = path.back();
  }
  for (int v : filled_vertices) {
    int h = to_position(v).first;
    int w = to_position(v).second;
    debug_board[h][w] = 'V';
  }
  // for (string s: debug_board) {
  //   cerr << s << endl;
  // }
  return tour;
  
  // set<int> required_vertices;
  // // REP(w, W) REP(h, H) {
  // //   if (ai.board[h][w] != '#' && visited_vertices.count(to_index(h, w)) == 0) {
  // //     required_vertices.insert(to_index(h, w));
  // //     covered_vertices.insert(to_index(h, w));
  // //     cover(to_index(h, w), ai, covered_vertices);
  // //   }
  // // }

  // // vector<string> debug_board= ai.board;
  // // for (int index: required_vertices) {
  // //   int h = to_position(index).first;
  // //   int w = to_position(index).second;
  // //   debug_board[h][w] = 'R';
  // // }
  // // for (string s: debug_board) {
  // //   cerr << s << endl;
  // // }
  // // required_vertices.insert(start_index);
  // vector<Edge> edges;
  // // for (int v: required_vertices) {
  // //   set<int> neighbors = gd.enumerate_neighbors(v, 10);
  // //   for (int w: neighbors) {
  // //     if (required_vertices.count(w)) {
  // //       edges.push_back(Edge(v, w, gd.shortest_path(v, w)));
  // //     }
  // //   }
  // // }

  // sort(ALL(edges), [](const Edge &e1, const Edge &e2) { return e1.weight < e2.weight; } );
  // vector<Edge> mst_edges;
  // UnionFind uf(MAX_H * MAX_W);

  // for (const auto &e: edges) {
  //   if (!uf.same(e.src_index, e.dst_index)) {
  //     mst_edges.push_back(e);
  //     uf.unite(e.src_index, e.dst_index);
  //   }
  // }
  // LOG_IF(FATAL, uf.size(edges[0].src_index) != int(required_vertices.size()));
  // cerr << "MST-size: " << mst_edges.size() << endl;
  
  // vector<vector<Edge>> G(MAX_V);
  // for (const auto &e: mst_edges) {
  //   G[e.src_index].push_back(e);
  //   G[e.dst_index].push_back(e.reverse());
  // }

  // return compute_traversal(start_index, G);
}

string move(int src_index, int dst_index, set<int> &visited_vertices, set<int> &wrapped_vertices, AI &ai, GraphDistance &gd) {
  const string command = get_move(gd, src_index, dst_index);
  int curr_index = src_index;
  for (char c: command) {
    ai.move(*char_to_direction(c));
    curr_index = move_index(curr_index, c);
    visited_vertices.insert(curr_index);
    cover(curr_index, ai, wrapped_vertices);
  }
  LOG_IF(FATAL, curr_index != dst_index);
  return command;
}

string enjoy_tour(int start_index, const vector<int> &tour, set<int> &visited_vertices, set<int> &wrapped_vertices, AI &ai,  GraphDistance &original_gd) {
  string res;
  int prev_index = start_index;
  size_t initial_i = start_index != tour[0] ? 0 : 1;

  for (size_t i = initial_i; i < tour.size(); i++) {
    int next_index = tour[i];
    const string command = get_move(original_gd, prev_index, next_index);
    int curr_index = prev_index;
    for (char c: command) {
      curr_index = move_index(curr_index, c);
      visited_vertices.insert(curr_index);
      cover(curr_index, ai, wrapped_vertices);
    }
    LOG_IF(FATAL, curr_index != next_index);
    res += command;
    visited_vertices.insert(next_index);
    prev_index = next_index;
  }
  return res;
}


int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  AI ai;
  // find start index
  vector<int> c_vertices;
  vector<int> x_vertices;
  vector<int> b_vertices;
  const int H = ai.board.size();
  const int W = ai.board[0].size();
  int start_index = to_index(ai.get_pos().first, ai.get_pos().second);
  REP(h, H) REP(w, W) {
    if (ai.board[h][w] == 'C') {
      c_vertices.push_back(to_index(h, w));
    }

    if (ai.board[h][w] == 'X') {
      x_vertices.push_back(to_index(h, w));
    }

    if (ai.board[h][w] == 'B') {
      b_vertices.push_back(to_index(h, w));
    }
  }
  LOG_IF(FATAL, start_index < 0);
  

  GraphDistance original_gd(MAX_V);
  int dh[] = {-1, 0, 1, 0};
  int dw[] = {0, -1, 0, 1};

  REP(h, H) REP(w, W) {
    if (ai.board[h][w] == '#') continue;
    
    REP(k, 4) {
      int nh = h + dh[k];
      int nw = w + dw[k];
      if (0 <= nh && nh < H && 0 <= nw && nw < W && ai.board[nh][nw] != '#') {
        original_gd.add_edge(to_index(h, w), to_index(nh, nw), 1);
      }
    }
  }

  string res = "";
  set<int> visited_vertices;
  set<int> wrapped_vertices;
  visited_vertices.insert(start_index);
  cover(start_index, ai, wrapped_vertices);
  int extensions_per_wrapper = (b_vertices.size()) / (c_vertices.size() + 1);

  vector<int> item_vertices;
  if (extensions_per_wrapper > 0) {
    cerr << "LET's use exntension (" << extensions_per_wrapper << " per wrapper)" << endl;
    for (int v: b_vertices) item_vertices.push_back(v);
  }

  if (!c_vertices.empty() && !x_vertices.empty()) {
    for (int v: c_vertices) item_vertices.push_back(v);
  }

  if (item_vertices.size() > 0) {
    vector<pair<int, int>> item_collect_moves = collect_item_greedy(start_index, original_gd, item_vertices);
    for (const auto &e: item_collect_moves) {
      int dst_index = e.second;
      res += move(start_index, dst_index, visited_vertices, wrapped_vertices, ai, original_gd);
      start_index = dst_index;
    }
  }

  if (!c_vertices.empty() && !x_vertices.empty()) {
    int best_d = 1e9;
    int best_x = -1;
    for (int v: x_vertices) {
      int spd = original_gd.shortest_path(start_index, v);
      if (best_d > spd) {
        best_d = spd;
        best_x = v;
      }
    }
    
    res += move(start_index, best_x, visited_vertices, wrapped_vertices, ai, original_gd);
    start_index = best_x;
    cerr << "++++++ LET'S CLONE (" << c_vertices.size() <<  ")++++++" << " " << res << endl;
  }

  for (int i = 0; i < extensions_per_wrapper; i++) {
    LOG_IF(FATAL, !ai.use_extension(0, 2 + i));
  }
  LOG_IF(FATAL, start_index != to_index(ai.get_pos()));

  cerr << "#B: " << b_vertices.size() << endl;
  cerr << "#C: " << c_vertices.size() << endl;
  cerr << "#visited_vertices: " << visited_vertices.size() << endl;
  vector<int> tour = compute_tour(start_index, ai, original_gd);
  
  if (c_vertices.empty() || x_vertices.empty() || tour.size() == 1) {
    REP(i, extensions_per_wrapper) {
      res += "B(" + to_string(2 + i) +  "," +  "0)";
    }
    cerr << "Tour Length: " << tour.size() << endl;
    res += enjoy_tour(start_index, tour, visited_vertices, wrapped_vertices, ai, original_gd);
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
      REP(i, extensions_per_wrapper) {
        res += "B(" + to_string(2 + i) +  "," +  "0)";
      }
      res += enjoy_tour(start_index, subtours[i], visited_vertices, wrapped_vertices, ai, original_gd);
    }
  }
  cerr << res.size() << endl;
  cout << res << endl;
  return 0;
}
