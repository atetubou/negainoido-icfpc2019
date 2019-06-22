#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <set>
#include <stdint.h>


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
  int shortest_path(int src, int dst, std::vector<int> &paths);
  std::set<int> enumerate_neighbors(int src, int limit);
 private:
  void clear_updates(const std::vector<int> &updated_vertices);
};



#endif /* GRAPH_H */

