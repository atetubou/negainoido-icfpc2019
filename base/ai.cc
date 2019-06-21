#include <iostream>
#include <cassert>
#include "base/base.h"
#include "base/ai.h"
#include "base/geometry.h"
#include "absl/strings/str_split.h"

Worker::Worker(Position pos) {
  current_pos = pos;
}


bool AI::reachable(Position target) {
  uint32_t x = target.first;
  uint32_t y = target.second;
  if (!(0 <= x && x < h && 0 <= y && y < w))
    return false;

  if (board[x][y] == '#')
    return false;

  auto worker_pos = get_pos();
  uint32_t wx = worker_pos.first;
  uint32_t wy = worker_pos.second;
  for (uint32_t cx = std::min(x, wx); cx <= std::max(x, wx); cx++) {
    for (uint32_t cy = std::min(y, wy); cy <= std::max(y, wy); cy++) {
      if (board[cx][cy] != '#')
        continue;

      Line l({wx + 0.5, wy + 0.5}, {x + 0.5, y + 0.5});

      Point cell(cx+0.5, cy+0.5);
      if (intersectSP(l, cell))
        return false;

      for(int i = 0; i < 4; i++) {
        const uint32_t dx[] = {cx, cx+1, cx+1, cx};
        const uint32_t dy[] = {cy, cy, cy+1, cy+1};
        Line edge({(double)dx[i], (double)dy[i]}, {(double)dx[(i+1)%4], (double)dy[(i+1)%4]});

        if (x == 0 && y == 2 && cx == 0 && cy ==1)
          std::cout<< l[0] << " " << l[1] << " " << edge[0]<< " " << edge[1] << std::endl;

        if (intersectSS(l, edge))
          return false;
      }
    }
  }

  return true;
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
    fill_cell(std::make_pair(x, y));
  }
}

bool AI::fill_cell(Position pos) {
  uint32_t x = pos.first;
  uint32_t y = pos.second;
  if (filled[x][y]) {
    return false;
  }
  if (!reachable(pos)) {
    return false;
  }
  filled[x][y] = true;
  switch(board[x][y]) {
    case 'B':
      worker.count_extension += 1;
      break;
    case 'F':
      worker.count_fast += 1;
      break;
    case 'L':
      worker.count_drill += 1;
      break;
    default:
      assert(false);
      break;
  }
  return true;
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
