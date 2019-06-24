#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>

#include <vector>
#include <set>
#include <string>
#include <functional>

#include "base/ai.h"

class GraphDistance {
  struct Edge {
    int src;
    int dst;
    int weight;
    Edge(const int src, const int dst, int weight) :
      src(src), dst(dst), weight(weight) { }
  };

  std::vector<std::vector<Edge>> graph;
  std::vector<int> distance;
  std::vector<int> prev;

 public:
  GraphDistance(uint32_t V);
  void add_edge(int src, int dst, int weight);
  int shortest_path(int src, int dst);
  int shortest_path(int src, int dst, std::vector<int> &paths, int stop_value = (1<<29));
  std::set<int> enumerate_neighbors(int src, int limit);
  void shortest_path_tree(int src, std::vector<int> &dist, std::vector<int> &parent);
  // return a pair of the shortest distance D and vertices whose distances from the source equals  D
  std::pair<int, std::set<int>> find_closest_vertices(int src, std::function<bool(int)> predicate);
 private:
  void clear_updates(const std::vector<int> &updated_vertices);
};


class GridGraph {
public:
  GridGraph(const std::vector<std::string>& board);

  using pos = std::pair<int, int>;

  int shortest_path(int sx, int sy, int gx, int gy);
  int shortest_path(int sx, int sy, int gx, int gy,
		    std::vector<pos> &path,
                    int stop_value = (1<<29));

  std::vector<Direction> shortest_paths(pos start, const std::vector<pos>& goals) const;

  bool can_visit(int x, int y) const;

  int to_graph_node(int x, int y);

  // returns (x, y) in grid from graph node.
  std::pair<int, int> to_grid_node(int v);

  static Direction move_to_action(const pos& a, const pos& b);
  static std::vector<Direction> path_to_actions(const std::vector<pos>& path);

private:
  std::vector<std::string> board_;
  GraphDistance graph_;
};

#endif /* GRAPH_H */
