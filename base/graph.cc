#include <algorithm>
#include <iostream>
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

int GraphDistance::shortest_path(int src, int dst, std::vector<int> &paths, int stop_value) {
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
    if (d > stop_value) {
      clear_updates(updated_vertices);
      return (1<<29);
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

void GraphDistance::shortest_path_tree(int src, std::vector<int> &dist, std::vector<int> &parents) {
  dist.clear();
  dist.resize(graph.size());
  parents.clear();
  parents.resize(graph.size());
  std::fill(dist.begin(), dist.end(), -1);
  std::fill(parents.begin(), parents.end(), -1);

  std::priority_queue<P, std::vector<P>, std::greater<P>>  que;
  que.push(P(0, src));

  dist[src] = 0;
  while (!que.empty()) {
    P p = que.top(); que.pop();
    int d = p.first;
    int v = p.second;

    if (d > dist[v]) {
      continue;
    }

    for (const auto &e: graph[v]) {
      int w = e.dst;
      int dw = dist[w];
      if (dw == -1 || dw > d + e.weight) {
        dist[w] = d + e.weight;
        parents[w] = v;
        que.push(P(dist[w], w));
      }
    }
  }
}


void GraphDistance::clear_updates(const std::vector<int> &updated_vertices) {
  for (auto v: updated_vertices) {
    distance[v] = -1;
    prev[v] = -1;
  }
}

int GridGraph::to_graph_node(int x, int y){
  const int h = board_[0].size();
  return x * h + y;
}

std::pair<int, int> GridGraph::to_grid_node(int v) {
  const int h = board_[0].size();
  return {v / h, v % h};
}

GridGraph::GridGraph(const std::vector<std::string>& board)
  : board_(board), graph_(board.size() * board[0].size()) {
  const int w = board.size();
  const int h = board[0].size();

  const int dx[] = {0, 0, 1, -1};
  const int dy[] = {1, -1, 0, 0};

  for (int i = 0; i < w; ++i) {
    for (int j = 0; j < h; ++j) {
      if (board_[i][j] == '#') {
	continue;
      }

      int cv = to_graph_node(i, j);

      for (int k = 0; k < 4; ++k) {
	int nx = i + dx[k];
	int ny = j + dy[k];
	if (nx < 0 || ny < 0 || nx >= w || ny >= h || board_[nx][ny] == '#') {
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
			     std::vector<std::pair<int, int>> &path,
                             int stop_value) {
  std::vector<int> gpath;
  int src = to_graph_node(sx, sy);
  int dst = to_graph_node(gx, gy);

  int ret = graph_.shortest_path(src, dst, gpath, stop_value);

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
    return Direction::Down;
  }

  if (a.first - 1 == b.first) {
    return Direction::Up;
  }

  if (a.second + 1 == b.second) {
    return Direction::Right;
  }

  if (a.second - 1 == b.second) {
    return Direction::Left;
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

bool GridGraph::can_visit(int x, int y) const {
  return
    0 <= x && x < static_cast<int>(board_.size()) &&
    0 <= y && y < static_cast<int>(board_[0].size()) &&
    board_[x][y] != '#';
}

std::vector<Direction> GridGraph::shortest_paths(pos start, const std::vector<pos>& goals) const {
  pos goal;
  return shortest_paths(start, goals, &goal);
}

std::vector<Direction> GridGraph::shortest_paths(pos start, const std::vector<pos>& goals,
						 pos* goal) const {
  const int dx[] = {0, 0, 1, -1};
  const int dy[] = {1, -1, 0, 0};

  const int sx = start.first;
  const int sy = start.second;

  std::vector<std::vector<int>> costs(board_.size(),
				      std::vector<int>(board_[0].size(), -1));
  std::queue<pos> q;
  for (const auto& g : goals) {
    if (start == g) {
      return std::vector<Direction>();
    }
    q.push(g);
    costs[g.first][g.second] = 0;
  }

  while (!q.empty()) {
    auto cur = q.front();
    q.pop();
    const int cx = cur.first;
    const int cy = cur.second;

    for (int i = 0; i < 4; ++i) {
      int nx = cx + dx[i];
      int ny = cy + dy[i];
      if (!can_visit(nx, ny) || costs[nx][ny] != -1) {
	continue;
      }
      costs[nx][ny] = costs[cx][cy] + 1;

      if (costs[sx][sy] != -1) {
	break;
      }

      q.push({nx, ny});
    }

    if (costs[sx][sy] != -1) {
      break;
    }
  }

  LOG_IF(FATAL, costs[sx][sy] == -1) << "unreachable from " << start << " to " << goals;

  std::vector<pos> path;

  for (;;) {
    path.push_back(start);
    if (costs[start.first][start.second] == 0) {
      break;
    }

    for (int i = 0; i < 4; ++i) {
      int nx = start.first + dx[i];
      int ny = start.second + dy[i];
      if (can_visit(nx, ny) && costs[nx][ny] != -1 &&
	  costs[nx][ny] < costs[start.first][start.second]) {
	start = {nx, ny};
	break;
      }
    }
  }

  *goal = start;

  return path_to_actions(path);
}
