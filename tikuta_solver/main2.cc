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

  const auto& order = tikutaOrder(ai, sqrt(ai.board.size() * ai.board[0].size() - ai.get_block_count()));

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
    // LOG(INFO) << s << " " << g << " " << spath;
    std:: cout << spath;
  }
  std::cout << std::endl;
}
