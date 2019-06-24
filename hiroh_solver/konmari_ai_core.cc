#include <unistd.h>

#include <glog/logging.h>

#include <queue>
#include <utility>
#include <memory>
#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "base/ai.h"
#include "base/graph.h"
#include "absl/types/optional.h"
#include "hiroh_solver/konmari_ai_solver.h"

#define MAX_L 500

class KonmariAI : public AI {
public:
  KonmariAI(const std::vector<std::string>& board,
            const std::vector<std::vector<bool>>& filled) :
    AI(board, filled),
    graph(board) {

    std::map<char, int> count;
    for (int x = 0; x < get_height(); x++) {
      for (int y = 0; y < get_width(); y++) {
        if (board[x][y] != '#' && board[x][y]) {
          count[board[x][y]]++;
        }
      }
    }

    DLOG(INFO) << "Number of clones: " << count['C'];
    DLOG(INFO) << "Number of drills: " << count['L'];
    DLOG(INFO) << "Number of extensions: " << count['B'];
    DLOG(INFO) << "Number of teleports: " << count['R'];
  }

  void set_drill_threshold(int v) {
    drill_threshold = v;
  }

  void pick_up_extensions();
  absl::optional<Position> decide_extension_pos();
  void try_to_use_extensions();
  void try_to_use_fast_weel();
  void try_to_use_teleport(std::vector<std::pair<int,int>>* path);
  bool try_to_use_drill(std::vector<std::pair<int,int>>* path);

  void get_next_item_if_exist();
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

  std::string path_to_string(const std::vector<Position>& p) {
    std::string ret;
    for (size_t i = 0; i < p.size(); i++) {
      if (i)
        ret += "->";
      ret += "(" + std::to_string(p[i].first) + "," + std::to_string(p[i].second) + ")";
    }
    return ret;
  }

  std::vector<Position> DPTSP(std::vector<Position> cells, int* cost);
  std::vector<Position> greedyTSP(std::vector<Position> cells, int* cost);

  int used_extension = 0;
  int drill_threshold = 7;
  GridGraph graph;
};

#define INF (1<<29)

namespace dp {

int memo[18][1<<18];
int memo2[18][1<<18];
int dp_cost[20][20] = {};

int dp_go(int cur, int passed, const int N) {
    if (passed == (1<<N) - 1) {
      return 0;
    }
    if (memo[cur][passed] > 0) {
      return memo[cur][passed];
    }
    int cur_cost = INF;
    int min_i = -1;
    for (int i = 0; i < N; i++) {
      if (passed & (1<<i))
        continue;
      int c = dp_cost[cur][i] + dp_go(i, passed ^ (1<<i), N);
      if (cur_cost > c) {
        cur_cost = c;
        min_i = i;
      }
    }
    memo[cur][passed] = cur_cost;
    memo2[cur][passed] = min_i;
    return cur_cost;
  };
}

std::vector<Position> KonmariAI::DPTSP(std::vector<Position> cells, int* cost) {
  cells.push_back(get_pos());
  const int st = cells.size() - 1;
  memset(dp::memo, -1, sizeof(dp::memo));
  for (size_t i = 0; i < cells.size(); i++) {
    for (size_t j = i + 1; j < cells.size(); j++) {
      dp::dp_cost[i][j] = dp::dp_cost[j][i] = graph.shortest_path(cells[i].first, cells[i].second,
  cells[j].first, cells[j].second);
    }
  }

  int cost0 = dp::dp_go(st, (1<<st), cells.size());
  std::vector<int> best_indices = {st};
  std::vector<Position> best_path;
  int passed = (1 << st);
  *cost = 0;
  while(best_indices.size() != cells.size()) {
    int next = dp::memo2[best_indices.back()][passed];
    *cost += dp::dp_cost[best_indices.back()][next];
    best_indices.push_back(next);
    passed ^= (1<<next);
    best_path.push_back(cells[next]);
  }

  if (cost0 != *cost) {
    LOG(FATAL) << "DPTSP is wrong...";
  }

  return best_path;
}

std::vector<Position> KonmariAI::greedyTSP(std::vector<Position> cells,
                                           int* cost) {
  std::vector<Position> best_path;
  Position cur = get_pos();
  std::set<Position> points(cells.begin(), cells.end());
  *cost = 0;
  while (!points.empty()) {
    int min_cost = INF;
    Position next = Position(-1, -1);
    for (const auto& dst : points) {
      int c = graph.shortest_path(cur.first, cur.second,
                                  dst.first, dst.second);
      if (min_cost > c) {
        min_cost = c;
        next = dst;
      }
    }
    best_path.push_back(next);
    cur = next;
    *cost += min_cost;
    points.erase(next);
  }

  return best_path;
}

void KonmariAI::try_to_use_fast_weel() {
  if (get_duration_fast() > 0)
    return;
  if (get_count_fast() > 0)
    use_fast_wheel();
}

void KonmariAI::try_to_use_teleport(std::vector<std::pair<int,int>>* path) {
  if (get_count_teleport() > 0) {
    install_beacon();
    DLOG(INFO) << "Put " << beacon_pos.size() << "th beacon!!";
  }
  int cur_cost = path->size() - 1;
  int cost_without_beacon = cur_cost;
  Position dst = path->back();
  for (const auto& beacon : beacon_pos) {
    std::vector<std::pair<int,int>> new_path;
    int c = graph.shortest_path(beacon.first, beacon.second,
                                dst.first, dst.second,
                                new_path, cur_cost);
    if (cur_cost > c + 1) {
      cur_cost = c + 1;
      *path = std::move(new_path);
    }
  }

  if (cur_cost != cost_without_beacon) {
    jump_to_beacon(path->front());
    DLOG(INFO) << "Jump! Saved cost is " << cost_without_beacon - cur_cost;
  }
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

  bool should_use_drill = cost_diff > drill_threshold;
  if (!should_use_drill)
    return false;

  std::vector<std::pair<int,int>> new_path;
  new_path.push_back(s);
  // Use drill!
  DLOG(INFO) << "Use Drill!";
  DLOG(INFO) << s.first << "," << s.second << "->"  << g.first << "," << g.second;
  DLOG(INFO) << "cost diff=" << cost_diff;
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
    DLOG(INFO) << "Use extension!";
    cur_pos_item = 0;
  }
}

void KonmariAI::pick_up_extensions() {
  std::vector<Position> extensions;
  for (int x = 0; x < get_height(); x++) {
    for (int y = 0; y < get_width(); y++) {
      if (board[x][y] == 'B')
        extensions.emplace_back(x, y);
    }
  }

  if (extensions.empty()) {
    return;
  }

  std::vector<Position> best_path;
  if (extensions.size() <= 15) {
    int greedy_cost = 0, best_cost = 0;
    std::vector<Position> greedy_path;
    best_path = DPTSP(extensions, &best_cost);
    greedy_path = greedyTSP(extensions, &greedy_cost);
    if (greedy_cost < best_cost) {
      LOG(FATAL) << "TSP computation is wrong....";
    }
    DLOG(INFO) << "DPTSP cost: " << best_cost;
    DLOG(INFO) << "Greedy cost: " << greedy_cost;
    DLOG(INFO) << "DPTSP path: " << path_to_string(best_path);
    DLOG(INFO) << "Greedy path: " << path_to_string(greedy_path);
  } else {
    DLOG(INFO) << "The number of extensions" << extensions.size() << " is too large. Do greedy TSP.";
    int greedy_cost = 0;
    best_path = greedyTSP(extensions, &greedy_cost);
    DLOG(INFO) << "Greedy cost: " << greedy_cost;
    DLOG(INFO) << "Greedy path: " << path_to_string(best_path);
  }

  for (const auto& dst : best_path) {
    Position cur = get_pos();
    std::vector<std::pair<int,int>> path;
    graph.shortest_path(cur.first, cur.second,
                        dst.first, dst.second,
                        path);
    for (const Direction& dir : GridGraph::path_to_actions(path)) {
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
          return np;
        }
        que.push(np);
      }
    }
  }
  return Position(-1, -1);
}

void KonmariAI::get_next_item_if_exist() {
  // If the next cell is teleport or drill, get it.
  for (auto& p : get_neighbors(get_pos())) {
    if (board[p.first][p.second] == 'R' ||
        board[p.first][p.second] == 'L') {
      Position cur_p = get_pos();
      move(GridGraph::move_to_action(cur_p, p));
      // Back to original position in order to not break shortest path move.
      move(GridGraph::move_to_action(p, cur_p));
    }
  }
}

void KonmariAI::konmari_move() {
  try_to_use_extensions();
  // Fast wheel is dangerous!!!
  // try_to_use_fast_weel();
  std::vector<std::pair<int,int>> path;
  auto dst = get_nearest_unfilled(&path);
  try_to_use_teleport(&path);
  bool used_drill = try_to_use_drill(&path);
#if 0
  if (!used_drill) {
    // If drill is not used, the path is usual. We can use shortest_filling_commands.
    auto commands = shortest_filling_commands(dst);
    for (const Command& cmd : commands) {
      if (filled[dst.first][dst.second])
        break;
      do_command(cmd);
      get_next_item_if_exist();
    }
  } else {
    for (const Direction& dir : GridGraph::path_to_actions(path)) {
      // Finish if all cells are already filled (due to fill by body).
      if (filled[dst.first][dst.second])
        break;
      move(dir);
      get_next_item_if_exist();
    }
  }
#else
  for (const Direction& dir : GridGraph::path_to_actions(path)) {
    // Finish if all cells are already filled (due to fill by body).
    if (filled[dst.first][dst.second])
      break;
    move(dir);
    get_next_item_if_exist();
  }
#endif

  if (used_drill) {
    // If drill is used, some walls in map become path.
    // Update grid graph to use the path to compute shortest path in the future.
    graph = GridGraph(board);
  }
}

/* KonmariAISolver */
class KonmariAISolver::Impl {
public:
  Impl(const std::vector<std::string>& board,
       const std::vector<std::vector<bool>>& filled);
  std::vector<std::vector<Command>> solve();
private:
  std::vector<std::string> board;
  std::vector<std::vector<bool>> filled;
};

KonmariAISolver::Impl::Impl(const std::vector<std::string>& board,
                            const std::vector<std::vector<bool>>& filled)
  : board(board), filled(filled) {}

std::vector<std::vector<Command>> KonmariAISolver::Impl::solve() {
  std::cerr << "Worker computing...." << std::endl;
  KonmariAI original_ai(board, filled);

#if 1
  Position init_pos = original_ai.get_pos();
  original_ai.pick_up_extensions();
  while (!original_ai.is_finished()) {
    original_ai.konmari_move();
    original_ai.dump_state();
  }
  int score = original_ai.get_time();
  std::cerr << "Score: " << score << std::endl;
  return original_ai.executed_cmds;
#else
  int best_score = (1<<29);
  std::string best_str;
  std::vector<std::vector<Command>> best_cmds;
  int best_v = -1;
  for (int v = 1; v < 30; v++) {
    KonmariAI ai = original_ai;
    std::cerr << "trying v=" << v << std::endl;
    ai.set_drill_threshold(v);
    // First pick up all extensions.
    ai.pick_up_extensions();
    while (!ai.is_finished()) {
      ai.konmari_move();
    }
    int score = ai.get_time();
    std::cerr << "Score (v=" << v << "):" << score << std::endl;
    if (best_score > score) {
      best_v = v;
      best_score = score;
      best_cmds = ai.executed_cmds;
    }
  }

  std::cerr << "Best Score (v=" << best_v << "):" << best_score << std::endl;
  return best_cmds;
#endif
}

KonmariAISolver::KonmariAISolver(const std::pair<int,int>& initial,
                                 const std::vector<std::string>& board,
                                 const std::vector<std::vector<bool>>& filled,
                                 const std::set<std::pair<int,int>>& area) {
  auto new_board = board;
  int W = board[0].size();
  int H = board.size();
  std::vector<std::vector<bool>> new_filled = filled;
  for (int i = 0; i < H; i++) {
    for (int j = 0; j < W; j++) {
      if (board[i][j] == '#' && filled[i][j]) {
        LOG(FATAL) << "Wall is filled....";
      }
    }
  }
  for (int i = 0; i < H; i++) {
    for (int j = 0; j < W; j++) {
      if (new_board[i][j] != '#') {
        if (area.find(std::make_pair(i,j)) == area.end()) {
          // This cell doesn't have to be filled by this ai.
          new_filled[i][j] = true;
          new_board[i][j] = '.';
        }
      }
    }
  }

  for (const auto& p : area) {
    if (new_board[p.first][p.second] == '#') {
      LOG(FATAL) << "wall is specified as filled area";
    }
  }

  new_board[initial.first][initial.second] = 'W';

  impl = std::make_unique<KonmariAISolver::Impl>(std::move(new_board), std::move(new_filled));
}

KonmariAISolver::~KonmariAISolver() = default;

std::vector<std::vector<Command>> KonmariAISolver::solve() {
  return impl->solve();
}
