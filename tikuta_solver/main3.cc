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

  while (ai.get_count_clone()) {
    const int workers = ai.get_count_active_workers();
    ai.use_clone(0);
    for (int i = 1; i < workers; ++i) {
      ai.nop(i);
    }
  }

  auto calc_groups = [&](){
    std::vector<pos> selected;
    for (auto i = 0u; i < ai.board.size(); ++i) {
      for (auto j = 0u; j < ai.board[i].size(); ++j) {
	if (ai.board[i][j] != '#' && !ai.filled[i][j]) {
	  selected.emplace_back(i, j);
	}
      }
    }

    std::mt19937 get_rand_mt;
    absl::c_shuffle(selected, get_rand_mt);
    
    if (static_cast<int>(selected.size()) > ai.get_count_active_workers()) {
      selected.resize(ai.get_count_active_workers());		       
    }
    auto g = get_groups(ai, selected);
    g.resize(ai.get_count_active_workers());
    return g;
  };

  auto groups = calc_groups();

  std::vector<std::deque<Direction>> directions(ai.get_count_active_workers());
  int get_filled_count = ai.get_filled_count();

  while (!ai.is_finished()) {
    for (int i = 0; i < ai.get_count_active_workers(); ++i) {
      if (ai.is_finished()) {
	ai.print_commands();
	return 0;
      }

      if (directions[i].empty()) {
	for (auto it = groups[i].begin(); it != groups[i].end();){
	  if (ai.filled[it->first][it->second]) {
	    it = groups[i].erase(it);
	  } else {
	    ++it;
	  }
	}

	if (!groups[i].empty()) {
	  auto p = gridg.shortest_paths(ai.get_pos(i), groups[i]);
	  directions[i] = std::deque<Direction>(p.begin(), p.end());
	}
      }
      
      if (directions[i].empty()) {
	LOG_IF(INFO, !ai.nop(i)) << "nop failed";
	if (get_filled_count != ai.get_filled_count()) {
	  get_filled_count = ai.get_filled_count();
	  groups = calc_groups();
	  directions.clear();
	  directions.resize(ai.get_count_active_workers());
	}
	continue;
      }

      if (!ai.move(directions[i].front(), i)) {
	ai.dump_state();
	LOG(INFO) << directions[i];
	LOG(FATAL) << "failed to move " << directions[i].front() << " " << i;
      }
      directions[i].pop_front();
    }
  }

  ai.print_commands();
}
