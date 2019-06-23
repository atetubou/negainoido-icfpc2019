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

using pos = std::pair<int, int>;

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  // AI's constructor accepts a input file from stdin.
  AI ai;

  int divide = sqrt(ai.board.size() * ai.board[0].size() - ai.get_block_count());
  const auto& order = tikutaOrder(ai, std::max(200, divide));

  GridGraph gridg(ai.board);

  for (auto i = 1u; i < order.size(); ++i) {
    const auto s = ai.get_pos();
    const auto& g = order[i];
    if (ai.filled[g.first][g.second]) {
      continue;
    }

    std::vector<pos> path;
    gridg.shortest_path(s.first, s.second,
			g.first, g.second, path);
    auto actions = GridGraph::path_to_actions(path);
    for (auto action : actions) {
      if (ai.filled[g.first][g.second]) {
	continue;
      }
      LOG_IF(FATAL, !ai.move(action)) << "invalid";
    }
  }

  LOG(INFO) << ai.get_time();
  ai.print_commands();
}
