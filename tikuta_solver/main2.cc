#include <algorithm>
#include <queue>
#include <random>
#include <set>
#include <vector>
#include <deque>

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
  
  std::deque<std::pair<pos, bool>> q;
  for (auto p : tikutaOrder(ai, std::max(200, divide))) {
    q.push_back({p, true});
  }
  
  GridGraph gridg(ai.board);

  while (!q.empty()) {
    const auto s = ai.get_pos();
    const auto g = q.front().first;
    const bool deferable = q.front().second;
    if (ai.board[g.first][g.second] != 'B' &&
	ai.filled[g.first][g.second]) {
      q.pop_front();
      continue;
    }

    std::vector<pos> path;
    gridg.shortest_path(s.first, s.second,
			g.first, g.second, path);
    auto actions = GridGraph::path_to_actions(path);
    for (auto action : actions) {
      if (ai.board[g.first][g.second] != 'B' &&
	  ai.filled[g.first][g.second]) {
	q.pop_front();
	continue;
      }
      LOG_IF(FATAL, !ai.move(action)) << "invalid";

      if (ai.get_count_extension()) {
	for (int i = 0;; i++) {
	  if (ai.use_extension(0, i + 2)) {
	    break;
	  }
	}
      }

      auto manips = ai.get_absolute_manipulator_positions();
      int dx[] = {0, 0, 1, -1};
      int dy[] = {1, -1, 0, 0};
      for (const auto& manip : manips) {
	for (int i = 0; i < 4; ++i) {
	  int nx = manip.first + dx[i];
	  int ny = manip.second + dy[i];
	  if (nx < 0 || ny < 0 || 
	      nx >= ai.get_height() || ny >= ai.get_width() ||
	      ai.board[nx][ny] == '#' || ai.filled[nx][ny]) {
	    continue;
	  }

	  bool visit_next = true;

	  for (int j = 0; j < 4; ++j) {
	    int nnx = nx + dx[j];
	    int nny = ny + dy[j];
	    if (nnx < 0 || nny < 0 || 
		nnx >= ai.get_height() || nny >= ai.get_width() ||
		ai.board[nnx][nny] == '#') {
	      continue;
	    }

	    if (!ai.filled[nnx][nny]) {
	      visit_next = false;
	      break;
	    }
	  }	  
	  
	  if (visit_next) {
	    for (int j = 0; j < 4; ++j) {
	      int nnx = nx + dx[j];
	      int nny = ny + dy[j];
	      if (nnx < 0 || nny < 0 || 
		  nnx >= ai.get_height() || nny >= ai.get_width() ||
		  ai.board[nnx][nny] == '#') {
		continue;
	      }
	      LOG(INFO) << nnx << " " << nny;
	      if (!ai.filled[nnx][nny]) {
		visit_next = false;
		break;
	      }
	    }	  
	  
	    q.push_front({{nx, ny}, false});
	  }
	}
      }

      if (!q.empty() && q.front().first != g && deferable) {
	LOG(INFO) << "visit this " << q.front();
	ai.dump_state();
	// exit(0);
	break;
      }
    }
  }

  LOG(INFO) << ai.get_time();
  ai.print_commands();
}
