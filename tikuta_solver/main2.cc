#include <algorithm>
#include <queue>
#include <random>
#include <set>
#include <vector>

#include <glog/logging.h>
#include <glog/stl_logging.h>
#include <gflags/gflags.h>

#include "absl/strings/str_join.h"

#include "base/ai.h"
#include "base/graph.h"
#include "tailed/LKH3_wrapper.h"

DEFINE_string(LKH3path, "./tailed/LKH", "");

using pos = std::pair<int, int>;

std::vector<std::vector<pos>> get_groups(const AI& ai,
					 const std::vector<pos>& tsp_tours) {
  int w = ai.board.size();
  int h = ai.board[0].size();

  std::vector<std::vector<bool>> visited(w, std::vector<bool>(h));
  std::vector<std::vector<pos>> groups(tsp_tours.size());

  std::queue<std::pair<int, pos>> q;
  for (auto i = 0u; i < tsp_tours.size(); ++i) {
    q.push(std::make_pair(i, tsp_tours[i]));
  }

  int dx[] = {0, 0, 1, -1};
  int dy[] = {1, -1, 0, 0};

  while (!q.empty()) {
    auto cur = q.front();
    q.pop();
    auto curpos = cur.second;
    if (visited[curpos.first][curpos.second]) {
      continue;
    }
    visited[curpos.first][curpos.second] = true;
    groups[cur.first].push_back(curpos);

    for (int i = 0; i < 4; ++i) {
      int nx = curpos.first + dx[i];
      int ny = curpos.second + dy[i];
      if (nx < 0 || ny < 0 || nx >= w || ny >= h ||
	  ai.board[nx][ny] == '#' || visited[nx][ny]) {
	continue;
      }
      q.push({cur.first, {nx, ny}});
    }
  }

  return groups;
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  // AI's constructor accepts a input file from stdin.
  AI ai;

  {
    auto board = ai.board;
    auto p = ai.get_pos();
    board[p.first][p.second] = 'W';
    for (auto b : board) {
      LOG(INFO) << b;
    }
  }

  const AI init_ai = ai;

  auto tsp_tours = SolveShrinkedTSP(ai, 3, FLAGS_LKH3path);

  // return 0;

  LOG(INFO) << tsp_tours;

  auto groups = get_groups(ai, tsp_tours);

  std::vector<pos> order;
  for (auto group : groups) {
    if (group != groups[0]) {
      std::sort(group.begin(), group.end());
    } else {
      std::sort(group.begin()+1, group.end());
    }
    LOG(INFO) << "group " << group;
    for (const auto& p : group) {
      order.push_back(p);
    }
  }

  GridGraph gridg(ai.board);

  for (auto i = 0u; i + 1 < order.size(); ++i) {
    const auto& s = order[i];
    const auto& g = order[i+1];
    std::vector<pos> path;
    gridg.shortest_path(s.first, s.second,
			g.first, g.second, path);
    auto actions = GridGraph::path_to_actions(path);
    std::string spath;
    for (auto c : actions) {
      spath +=  direction_to_char(c);
    }
    LOG(INFO) << s << " " << g << " " << spath;
    std:: cout << spath;
  }
  std::cout << std::endl;
}
