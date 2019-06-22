#include <algorithm>
#include <queue>
#include <random>
#include <set>
#include <vector>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "absl/strings/str_join.h"

#include "base/ai.h"
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

  const AI init_ai = ai;

  auto tsp_tours = SolveShrinkedTSP(ai.board, 100, FLAGS_LKH3path);

  auto groups = get_groups(ai, tsp_tours);

}
