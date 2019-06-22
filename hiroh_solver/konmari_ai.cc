#include <unistd.h>

#include <glog/logging.h>

#include <queue>
#include <utility>
#include <memory>
#include <iostream>
#include <vector>
#include <map>

#include "base/ai.h"
#include "base/graph.h"
#include "absl/types/optional.h"

#define MAX_L 500

class KonmariAI : public AI {
public:
  KonmariAI() :
    AI(),
    graph(board) {}
  void pick_up_extensions();
  absl::optional<Position> decide_extension_pos();
  void try_to_use_extensions();
  void konmari_move();
  Position get_nearest_unfilled(std::vector<std::pair<int,int>>* path);

private:
  std::vector<Position> get_neighbors(const Position& p) {
    std::vector<Position> ret;
    constexpr int dx[] = {-1, 0, 0, 1};
    constexpr int dy[] = {0, -1, 1, 0};
    for (int i = 0; i < 4; i++) {
      int nx = p.first + dx[i];
      int ny = p.second + dy[i];
      if (nx >= 0 && nx < get_height() && ny >= 0 && ny < get_width()) {
        ret.emplace_back(nx, ny);
      }
    }
    return ret;
  }

  GridGraph graph;
};

absl::optional<Position> KonmariAI::decide_extension_pos() {
  std::set<Position> unfilled_cands;
  for (const auto& p : get_absolute_manipulator_positions()) {
    for (const auto& np: get_neighbors(p)) {
      int x = np.first;
      int y = np.second;
      if (!filled[x][y] &&
          unfilled_cands.find(np) == unfilled_cands.end()) {
        unfilled_cands.insert(np);
        if (reachable(np)) {
          return np;
        }
      }
    }
  }
  // TODO: optimize more here.
  if (unfilled_cands.size() > 0)
    return *unfilled_cands.begin();
  return absl::nullopt;
}

void KonmariAI::try_to_use_extensions() {
  while (get_count_extension() > 0) {
    auto ext_pos = decide_extension_pos();
    if (!ext_pos) {
      return;
    }
    if (!use_extension(ext_pos->first - get_pos().first,
                       ext_pos->second - get_pos().second)) {
      LOG(FATAL) << "Failed to use extension";
      return;
    }
  }
}

void KonmariAI::pick_up_extensions() {
  std::set<Position> extensions;
  for (int x = 0; x < get_height(); x++) {
    for (int y = 0; y < get_width(); y++) {
      if (board[x][y] == 'B')
        extensions.emplace(x, y);
    }
  }

  while(!extensions.empty()) {
    Position cur = get_pos();
    int min_cost = (1<<29);
    std::vector<std::pair<int,int>> shortest_path;
    Position next = Position(-1, -1);
    for (const auto& dst : extensions) {
      std::vector<std::pair<int,int>> path;
      int cost = graph.shortest_path(cur.first, cur.second,
                                     dst.first, dst.second,
                                     path);
      if (min_cost > cost) {
        shortest_path = std::move(path);
        min_cost = cost;
        next = dst;
      }
    }
    extensions.erase(next);
    // move cur -> next.
    // pick up extension.
    for (const Direction& dir : GridGraph::path_to_actions(shortest_path)) {
      move(dir);
    }
    if (get_count_extension() == 0) {
      LOG(FATAL) << "Failed to get extension....";
      return;
    }

    // use extension.
    try_to_use_extensions();
  }
}

Position KonmariAI::get_nearest_unfilled(std::vector<std::pair<int,int>>* path) {
  // Do BFS for the nearest path.
  bool passed [MAX_L][MAX_L] = {};
  std::queue<Position> que;
  const auto centre = get_pos();
  passed[centre.first][centre.second] = true;
  que.push(centre);
  while (!que.empty()) {
    auto p = std::move(que.front());
    que.pop();
    for (const auto& np : get_neighbors(p)) {
      int x = np.first;
      int y = np.second;
      if (!passed[x][y]) {
        passed[x][y] = true;
        if (!filled[x][y] && board[x][y] != '#') {
          graph.shortest_path(centre.first, centre.second, x, y, *path);
          return Position(x, y);
        }
        que.push(Position(x, y));
      }
    }
  }
  return Position(-1, -1);
}

void KonmariAI::konmari_move() {
  try_to_use_extensions();
  std::vector<std::pair<int,int>> path;
  auto dst = get_nearest_unfilled(&path);
  DLOG(INFO) << "dst=" << dst.first << " " << dst.second;
  DLOG(INFO) << path.size();
  // move: path[0] -> path[1].
  for (const Direction& dir : GridGraph::path_to_actions(path)) {
      move(dir);
  }
}

int main() {
  std::unique_ptr<KonmariAI> ai(new KonmariAI());
  // First pick up all extensions.
  ai->pick_up_extensions();
  while (!ai->is_finished()) {
    ai->konmari_move();

    // std::cerr << "State-------" << std::endl;
    // ai->dump_state();
  }
  ai->print_commands();
}
