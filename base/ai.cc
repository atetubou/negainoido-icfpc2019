#include <iostream>
#include "base/base.h"
#include "base/ai.h"
#include "absl/strings/str_split.h"

Worker::Worker(Position pos) {
  current_pos = pos;
}

AI::AI() {
  std::cin >> h >> w;
  std::string buf;
  while (std::cin >> buf) {
    board.push_back(buf);
  }
  Position worker_pos = std::make_pair(0, 0);
  for (uint32_t i = 0; i < h; ++i) {
    for (uint32_t j = 0; j < w; ++j) {
      if (board[i][j] == 'W') {
        worker_pos = std::make_pair(i, j);
      }
    }
  }
  filled.resize(h);
  for (uint32_t i = 0; i < h; ++i) filled[i].resize(w);

  worker = Worker(worker_pos);

  for (auto&p : worker.manipulator_range) {
    auto x = worker.current_pos.first + p.first;
    auto y = worker.current_pos.second + p.second;
    fill_cell(x, y);
  }
}

uint32_t AI::get_time() {
  return current_time;
}

Position AI::get_pos() {
  return worker.current_pos;
}

Direction AI::get_dir() {
  return worker.current_dir;
}

uint32_t AI::get_count_fast() { return worker.count_fast; }
uint32_t AI::get_count_drill() { return worker.count_drill; }
uint32_t AI::get_count_extension() { return worker.count_extension; }

void AI::turn_CW() {
  worker.current_dir =
    static_cast<Direction>( ( static_cast<int>(worker.current_dir) + 1 ) % 4 );
}
void AI::turn_CCW() {
  worker.current_dir =
    static_cast<Direction>( ( static_cast<int>(worker.current_dir) - 1 ) % 4 );
}
//
// public:
//   std::vector<Position> get_range();
//   bool move(const Direction &dir);
//   bool use_extension(const int dx, const int dy);
//   bool use_fast_wheel();
//   bool use_drill();
// };
