#include <algorithm>
#include <queue>

#include <glog/logging.h>
#include <glog/stl_logging.h>

#include "base/graph.h"

typedef std::pair<int, int> P;

GraphDistance::GraphDistance(uint32_t V) {
  graph.clear();
  graph.resize(V);

  distance.clear();
  distance.resize(V);
  fill(distance.begin(), distance.end(), -1);

  prev.clear();
  prev.resize(V);
  fill(prev.begin(), prev.end(), -1);
}

void GraphDistance::add_edge(int src, int dst, int weight) {
  graph[src].push_back(GraphDistance::Edge(src, dst, weight));
}

int GraphDistance::shortest_path(int src, int dst) {
  std::vector<int> paths;
  return shortest_path(src, dst, paths);
}

int GraphDistance::shortest_path(int src, int dst, std::vector<int> &paths) {
  paths.clear();

  int res = -1;
  std::vector<int> updated_vertices;
  std::priority_queue<P, std::vector<P>, std::greater<P>>  que;
  que.push(P(0, src));
  updated_vertices.push_back(src);
  distance[src] = 0;
  while (!que.empty()) {
    P p = que.top(); que.pop();
    int d = p.first;
    int v = p.second;

    if (v == dst) {
      res = d;
      while (v >= 0) {
        paths.push_back(v);
        v = prev[v];
      }
      break;
    }


    for (const auto &e: graph[v]) {
      int w = e.dst;
      int dw = distance[w];
      if (dw == -1 || dw > d + e.weight) {
        updated_vertices.push_back(w);
        distance[w] = d + e.weight;
        prev[w] = v;
        que.push(P(distance[w], w));
      }
    }
  }

  std::reverse(paths.begin(), paths.end());
  clear_updates(updated_vertices);
  return res;
}

std::set<int> GraphDistance::enumerate_neighbors(int src, int limit) {
  std::set<int> neighbors;
  std::vector<int> updated_vertices;
  std::priority_queue<P, std::vector<P>, std::greater<P>>  que;
  que.push(P(0, src));
  updated_vertices.push_back(src);

  distance[src] = 0;
  while (!que.empty()) {
    P p = que.top(); que.pop();
    int d = p.first;
    int v = p.second;

    if (v > 0) {
      neighbors.insert(v);
    }

    for (const auto &e: graph[v]) {
      int w = e.dst;
      int cdw = distance[w];
      int ndw = d + e.weight;
      if (ndw <= limit && (cdw == -1 || cdw > ndw)) {
        updated_vertices.push_back(w);
        distance[w] = ndw;
        prev[w] = v;
        que.push(P(distance[w], w));
      }
    }
  }
  clear_updates(updated_vertices);
  return neighbors;
}

void GraphDistance::clear_updates(const std::vector<int> &updated_vertices) {
  for (auto v: updated_vertices) {
    distance[v] = -1;
    prev[v] = -1;
  }
}

int GridGraph::to_graph_node(int x, int y){
  const int h = board_.size();
  return y * h + x;
}

std::pair<int, int> GridGraph::to_grid_node(int v) {
  const int h = board_.size();
  return {v % h, v / h};
}

GridGraph::GridGraph(const std::vector<std::string>& board)
  : board_(board), graph_(board.size() * board[0].size()) {
  const int h = board.size();
  const int w = board[0].size();

  const int dx[] = {0, 0, 1, -1};
  const int dy[] = {1, -1, 0, 0};

  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      if (board_[i][j] == '#') {
	continue;
      }

      int cv = to_graph_node(j, i);

      for (int k = 0; k < 4; ++k) {
	int nx = j + dx[k];
	int ny = i + dy[k];
	if (nx < 0 || ny < 0 || nx >= w || ny >= h || board_[ny][nx] == '#') {
	  continue;
	}

	int nv = to_graph_node(nx, ny);
	graph_.add_edge(cv, nv, 1);
      }
    }
  }
}

int GridGraph::shortest_path(int sx, int sy, int gx, int gy) {
  std::vector<std::pair<int, int>> path;
  return shortest_path(sx, sy, gx, gy, path);
}

int GridGraph::shortest_path(int sx, int sy, int gx, int gy,
			     std::vector<std::pair<int, int>> &path) {
  std::vector<int> gpath;
  int src = to_graph_node(sx, sy);
  int dst = to_graph_node(gx, gy);

  int ret = graph_.shortest_path(src, dst, gpath);

  for (auto v : gpath) {
    path.push_back(to_grid_node(v));
  }

  return ret;
}

/* static */
Direction GridGraph::move_to_action(const GridGraph::pos& a,
			       const GridGraph::pos& b) {
  LOG_IF(FATAL, abs(a.first - b.first) + abs(a.second - b.second) != 1)
    << "Invalid args " << a << " " << b;

  if (a.first + 1 == b.first) {
    return Direction::Right;
  }

  if (a.first - 1 == b.first) {
    return Direction::Left;
  }

  if (a.second + 1 == b.second) {
    return Direction::Up;
  }

  if (a.second - 1 == b.second) {
    return Direction::Down;
  }

  LOG(FATAL) << "Invalid args " << a << " " << b;
}

/* static */
std::vector<Direction> GridGraph::path_to_actions(const std::vector<pos>& path) {
  std::vector<Direction> actions;
  for (size_t i = 0; i + 1 < path.size(); ++i) {
    actions.push_back(move_to_action(path[i], path[i+1]));
  }
  return actions;
}
