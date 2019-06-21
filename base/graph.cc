#include "base/graph.h"
#include <algorithm>
#include <queue>

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
