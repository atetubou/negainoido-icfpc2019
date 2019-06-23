#include <algorithm>
#include <queue>
#include <random>
#include <set>
#include <vector>
#include <deque>

#include <glog/logging.h>
#include <glog/stl_logging.h>
#include <gflags/gflags.h>

#include "absl/algorithm/container.h"
#include "absl/strings/str_join.h"

#include "base/ai.h"
#include "base/graph.h"
#include "tailed/LKH3_wrapper.h"

using pos = std::pair<int, int>;

void collect_cloning(AI *ai, GridGraph* gridg) {
  auto cloning = ai->cloning_points;
  while (!cloning.empty()) {
    for (const auto d : gridg->shortest_paths(ai->get_pos(), cloning)) {
      ai->move(d);
    }

    auto it = absl::c_find(cloning, ai->get_pos());
    CHECK(it != cloning.end()) << ai->get_pos() << " " << cloning;
    cloning.erase(it);
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  // AI's constructor accepts a input file from stdin.
  AI ai;
  GridGraph gridg(ai.board);

  collect_cloning(&ai, &gridg);

  if (!ai.spawn_points.empty()) {
    for (const auto d : gridg.shortest_paths(ai.get_pos(), ai.spawn_points)) {
      ai.move(d);
    }
  }

  ai.print_commands();
}
