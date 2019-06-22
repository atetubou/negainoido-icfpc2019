#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <unistd.h>

#include <glog/logging.h>

#define MAX_L 500

int H, W;
char board[MAX_L][MAX_L];

std::vector<std::pair<int,int>> vertical_line[MAX_L]; // x is the same. (a, b) means the is a line (x, a) -> (x, b)

using P = std::pair<int,int>;

void printP(const P& p) {
  std::cout << "(" << p.first << "," << p.second << ")";
}

void squash(std::vector<std::pair<int,int>>* line) {
  if (line->empty())
    return;
  std::sort(line->begin(), line->end());

  std::vector<std::pair<int,int>> squashed_line;
  squashed_line.push_back((*line)[0]);
  for (size_t i = 1; i < line->size(); i++) {
    auto& p = squashed_line.back();
    if (p.second == (*line)[i].first) {
      p.second = (*line)[i].second;
    } else {
      squashed_line.push_back((*line)[i]);
    }
  }
  *line = std::move(squashed_line);
}

std::vector<P> create_from_edge(std::map<P, std::vector<P>> edge) {
  std::vector<P> ret;
  P cur = edge.begin()->first;
  P prev = cur;
  ret.push_back(cur);
  cur = edge[cur][1];
  edge.erase(prev);
  while (!edge.empty()) {
    ret.push_back(cur);
    P next;
    for (const auto& v : edge[cur]) {
      if (v != prev) {
        next = v;
        break;
      }
    }
    prev = cur;
    cur = next;
    edge.erase(prev);
  }
  return ret;
}


// board (x, y)
// board[W][H]
int main() {
  std::cin >> H >> W;
  std::map<char, std::vector<P>> special_points;
  for (int i = 0; i < H; i++) {
    std::string l;
    std::cin >> l;
    for (int j = 0; j < W; j++) {
      char c = l[j];
      if (c != '.' && c != '#') {
        special_points[c].emplace_back(j, H-i-1);
        c = '.';
      }
      board[j][H-1-i] = c;
    }
  }

  // Looks for vertical line.
  for (int y = 0; y < H; y++) {
    // ...####...# -> #|...|####|...|#
    for (int x = 0; x < W; x++) {
      if (x == 0) {
        if (board[x][y] == '.') {
          vertical_line[x].emplace_back(y, y+1);
        }
      } else{
        if (x == W - 1) {
          if (board[x][y] == '.') {
            vertical_line[x+1].emplace_back(y, y+1);
          }
        }
        if (board[x-1][y] == '.' && board[x][y] == '#') {
          // .#
          // (x-1)x
          vertical_line[x].emplace_back(y, y+1);
        } else if (board[x-1][y] == '#' && board[x][y] == '.') {
          // #.
          // (x-1)x
          vertical_line[x].emplace_back(y, y+1);
        }
      }
    }
  }

  std::map<P, std::vector<P>> edge;
  std::map<int, std::vector<int>> y_p;
  // Process for vertical line.
  for (int x = 0; x <= W; x++) {
    squash(&vertical_line[x]);
    // sorted
    for (size_t j = 0; j < vertical_line[x].size(); j++) {
      P p1 = P(x, vertical_line[x][j].first);
      P p2 = P(x, vertical_line[x][j].second);
      edge[p1].push_back(p2);
      edge[p2].push_back(p1);
    }
    for (const auto& p : vertical_line[x]) {
      y_p[p.first].push_back(x);
      y_p[p.second].push_back(x);
    }
  }

  for (const auto& t : y_p) {
    int y = t.first;
    const std::vector<int>& xs = t.second;
    for (int j = 0; j < (int)(xs.size()) - 1; j++) {
      if (j % 2 != 0)
        continue;
      P p1(xs[j], y);
      P p2(xs[j+1], y);
      edge[p1].push_back(p2);
      edge[p2].push_back(p1);
    }
  }

  auto unique_points = create_from_edge(std::move(edge));

  constexpr char delim = '#';
  // Output
  {
    auto it = unique_points.begin();
    for (size_t i = 0; i < unique_points.size(); i++) {
      if (i != 0)
        std::cout << ",";
      printP(*it);
      it++;
    }
  }
  std::cout << delim;

  if (special_points['W'].size() != 1) {
    LOG(FATAL) << "Worker's point is not unique";
    return -1;
  }

  printP(special_points['W'][0]);
  std::cout << delim;
  // No obstacle.
  std::cout << delim;

  special_points.erase('W');
  {
    auto it = special_points.begin();
    for (size_t i = 0; i < special_points.size(); i++) {
      if (i != 0)
        std::cout << ';';

      char c = it->first;
      const auto& points = it->second;
      for (size_t j = 0; j < points.size(); j++) {
        if (j != 0)
          std::cout << ";";
        std::cout << c;
        printP(points[j]);
      }
      it++;
    }
  }
  return 0;
}
