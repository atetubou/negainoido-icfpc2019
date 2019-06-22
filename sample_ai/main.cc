/*

The algorithm is same with tikuta_solver's one.

example usage:
$ bazel run //sample_ai:sample_ai < problems/prob-001.in
WWDSAWDDSAWDDS
*/


#include <iostream>
#include <string>
#include <set>
#include <vector>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "base/ai.h"

const Direction dirs[] = {
  Direction::Up,
  Direction::Down,
  Direction::Left,
  Direction::Right,
};

std::set<Position> visited;

void dfs(AI &ai) {
  visited.insert(ai.get_pos());
  if(ai.is_finished())
    return;

  for (int i = 0; i < 4; ++i) {
    Direction dir = dirs[i];
    auto next_pos = ai.get_neighbor(dir);
    if(!ai.try_move(dir)) {
      continue;
    }

    if(visited.find(next_pos) != visited.end()) {
      continue;
    }

    ai.move(dir);

    dfs(ai);

    if(ai.is_finished())
      return;

    ai.move(dirs[i^1]);
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();


  // AI's constructor accepts a input file from stdin.
  AI ai;

  dfs(ai);
  LOG_IF(FATAL, !ai.is_finished()) << "AI terminated before it visits all cells";

  ai.print_commands();
}
