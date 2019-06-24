#include "hiroh_solver/konmari_ai_solver.h"
#include "base/ai.h"
#include <iostream>

std::string commands2str(const std::vector<std::vector<Command>>& cmds,
                         int height) {
  std::string s;
  for (size_t i = 0; i < cmds.size(); ++i) {
    if (i != 0)
      s += "#";
    for (const auto& cmd : cmds[i]) {
      s += AI::cmd2str(cmd, height);
    }
  }
  return s;
}


int main() {
  // To read file.
  AI read_file_ai;
  std::set<std::pair<int, int>> area;

  int W = read_file_ai.get_width();
  int H = read_file_ai.get_height();
  for (int x = 0; x < H; x++) {
    for (int y = 0; y < W; y++) {
      if (read_file_ai.board[x][y] != '#')
        area.emplace(x, y);
    }
  }

  KonmariAISolver ai(read_file_ai.get_pos(),
                     read_file_ai.board,
                     read_file_ai.filled,
                     area);
  auto cmds = ai.solve();
  std::cout << commands2str(cmds, read_file_ai.get_height()) << std::endl;
}
