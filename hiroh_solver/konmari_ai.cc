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
  void try_to_use_fast_weel();
  bool try_to_use_drill(std::vector<std::pair<int,int>>* path);
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
      if (nx >= 0 && nx < get_height() && ny >= 0 && ny < get_width() && board[nx][ny] != '#') {
        ret.emplace_back(nx, ny);
      }
    }
    return ret;
  }

  int used_extension = 0;
  GridGraph graph;
};

void KonmariAI::try_to_use_fast_weel() {
  if (get_duration_fast() > 0)
    return;
  if (get_count_fast() > 0)
    use_fast_wheel();
}

bool KonmariAI::try_to_use_drill(std::vector<std::pair<int,int>>* path) {
  // We only use drill for one move.
  if (get_duration_drill() > 0)
    return false;
  if (get_count_drill() == 0)
    return false;

  int cost_without_drill = path->size() - 1;
  // path.begin() -> path.back().
  const auto& s = path->front();
  const auto& g = path->back();
  const int cost_with_drill = std::abs(g.first - s.first) + std::abs(g.second - s.second);
  // Lazy assumption. We may not be able to g with drill.
  if (cost_with_drill > 30)
    return false;
  const int cost_diff = cost_without_drill - cost_with_drill;

  bool should_use_drill = cost_diff > 5;
  if (!should_use_drill)
    return false;

  std::vector<std::pair<int,int>> new_path;
  new_path.push_back(s);
  // Use drill!
  LOG(INFO) << "Use Drill!";
  LOG(INFO) << s.first << "," << s.second << "->"  << g.first << "," << g.second;
  LOG(INFO) << "cost diff=" << cost_diff;
  use_drill();
  for (int i = 0; g.first != s.first + i;) {
    i += g.first > s.first ? 1 : -1;
    new_path.emplace_back(s.first+i, s.second);
  }
  for (int i = 0; g.second != s.second + i;) {
    i += g.second > s.second ? 1 : -1;
    new_path.emplace_back(g.first, s.second+i);
  }

  *path = std::move(new_path);
  return true;
}

absl::optional<Position> KonmariAI::decide_extension_pos() {
  auto ret = absl::make_optional<Position>(get_pos().first, get_pos().second+2+used_extension);
  used_extension++;
  return ret;
  /*
  std::set<Position> unfilled_cands;
  std::set<Position> manus;
  for (const auto& p : get_absolute_manipulator_positions()) {
    manus.insert(p);
  }

  for (const auto& p : get_absolute_manipulator_positions()) {
    for (const auto& np: get_neighbors(p)) {
      int x = np.first;
      int y = np.second;
      if (!filled[x][y] &&
          manus.find(np) == manus.end() &&
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
  */
}

void KonmariAI::try_to_use_extensions() {
  int cur_pos_item = board[get_pos().first][get_pos().second] == 'B' ? 1 : 0;

  while (cur_pos_item + get_count_extension() > 0) {
    auto ext_pos = decide_extension_pos();
    if (!ext_pos) {
      return;
    }
    if (!use_extension(ext_pos->first - get_pos().first,
                       ext_pos->second - get_pos().second)) {
      LOG(FATAL) << "Failed to use extension";
      return;
    }
    LOG(INFO) << "Use extension!";
    cur_pos_item = 0;
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
  // Fast wheel is dangerous!!!
  // try_to_use_fast_weel();
  std::vector<std::pair<int,int>> path;
  get_nearest_unfilled(&path);
  bool used_drill = try_to_use_drill(&path);
  for (const Direction& dir : GridGraph::path_to_actions(path)) {
    move(dir);
  }

  if (used_drill) {
    // If drill is used, some walls in map become path.
    // Update grid graph to use the path to compute shortest path in the future.
    graph = GridGraph(board);
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
  std::cerr << "Score:" << ai->get_time() << std::endl;
  ai->print_commands();
}
