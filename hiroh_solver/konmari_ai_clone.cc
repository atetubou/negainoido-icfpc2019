#include <algorithm>
#include <queue>
#include <random>
#include <set>
#include <vector>
#include <deque>
#include <memory>

#include <glog/logging.h>
#include <glog/stl_logging.h>
#include <gflags/gflags.h>

#include "absl/algorithm/container.h"
#include "absl/strings/str_join.h"

#include "base/ai.h"
#include "base/graph.h"
#include "tailed/LKH3_wrapper.h"
#include "hiroh_solver/konmari_ai_solver.h"

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

  if (!ai.spawn_points.empty() && ai.get_count_clone()) {
    for (const auto d : gridg.shortest_paths(ai.get_pos(), ai.spawn_points)) {
      ai.move(d);
    }
  }

  size_t nop_count[100] = {};
  // nop coun
  while (ai.get_count_clone()) {
    const int workers = ai.get_count_active_workers();
    ai.use_clone(0);
    for (int i = 1; i < workers; ++i) {
      nop_count[i]++;
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

  std::vector<std::unique_ptr<KonmariAISolver>> workers;
  for (const auto& g : calc_groups()) {
    workers.push_back(std::make_unique<KonmariAISolver>(ai.get_pos(),
                                                        ai.board,
                                                        ai.filled,
                                                        std::set<std::pair<int,int>>(g.begin(), g.end())));
  }

  LOG(ERROR) << "The number of workers: " << workers.size();
  const int height = ai.board.size();
  std::vector<std::vector<Command>> cmds(workers.size());
  size_t max_cmd_length = 0;
  std::vector<int> scores(workers.size());
  for (size_t i = 0; i < workers.size(); i++) {
    const auto& w = workers[i];
    for (size_t k = 0; k < nop_count[i]; k++) {
      struct Command nop_cmd;
      nop_cmd.type = CmdType::Nop;
      cmds[i].push_back(nop_cmd);
    }
    auto w_cmds = w->solve(&scores[i])[0];
    for (const auto& cmd : w_cmds) {
      cmds[i].push_back(cmd);
    }
    max_cmd_length = std::max(max_cmd_length, cmds.back().size());
  }

  std::string phase1 = ai.commands2str();
  std::string phase2;
  std::cout << phase1;
  for (size_t j = 0; j < workers.size(); j++) {
    for (size_t i = cmds[j].size(); i < max_cmd_length; i++) {
      struct Command nop_cmd;
      nop_cmd.type = CmdType::Nop;
      cmds[j].push_back(nop_cmd);
    }
    if (j != 0)
      std::cout << "#";
    for (const auto& cmd : cmds[j])
      std::cout << AI::cmd2str(cmd, height);
  }
  std::cout << std::endl;

  std::cerr << "Summarized Score: " << ai.get_time() + *std::max_element(scores.begin(), scores.end()) << std::endl;
}
