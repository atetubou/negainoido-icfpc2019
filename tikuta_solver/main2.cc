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

  auto tsp_tours = SolveShrinkedTSP(ai,
				    sqrt(ai.board.size() * ai.board[0].size() - ai.get_block_count()),
				    FLAGS_LKH3path);

  auto groups = get_groups(ai, tsp_tours);

  {
    auto board = ai.board;
    auto p = ai.get_pos();
    board[p.first][p.second] = 'W';
    for (auto i = 0u; i < groups.size(); ++i) {
      for (const auto& g : groups[i]) {
	auto& b = board[g.first][g.second];
	if (b == '.') b = 128 + i;
      }
    }

    for (const auto& row : board) {
      for (const auto& g : row) {
	if (g >= 0) {
	  std::cerr << g << g;
	} else {
	  char buf[8];
	  sprintf(buf, "%2d", static_cast<unsigned char>(g)-128);
	  std::cerr << buf;
	}
      }
      std::cerr << std::endl;
    }    
  }

  GridGraph gridg(ai.board);

  std::vector<pos> order;

  {
    std::vector<std::vector<bool>> visited(ai.board.size(),
					   std::vector<bool>(ai.board[0].size()));
    pos cur = groups[0][0];

    for (auto group : groups) {
      for(;;) {
	order.push_back(cur);
	visited[cur.first][cur.second] = true;
	bool end = true;

	int nearest = 1e9;
	pos ne;

	for (const auto& p : group) {
	  if (visited[p.first][p.second]) {
	    continue;
	  }
	  end = false;
	  int nc = gridg.shortest_path(cur.first, cur.second,
				       p.first, p.second);
	  if (nc < nearest) {
	    nearest = nc;
	    ne = p;
	  }
	}

	if (end) break;
	cur = ne;
      }
    }
  }


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
