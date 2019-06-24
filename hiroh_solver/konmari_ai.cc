#include "hiroh_solver/konmari_ai_solver.h"
#include "base/ai.h"

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
  std::cout << ai.solve();
}
