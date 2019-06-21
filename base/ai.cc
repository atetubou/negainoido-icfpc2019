#include <iostream>
#include <cassert>
#include "base/base.h"
#include "base/ai.h"
#include "base/geometry.h"
#include "absl/strings/str_split.h"

Worker::Worker(Position pos) {
  current_pos = pos;
}

bool AI::valid_pos(Position target) {
  uint32_t x = target.first;
  uint32_t y = target.second;
  return 0 <= x && x < h && 0 <= y && y < w;
}

bool AI::reachable(Position target) {
  uint32_t x = target.first;
  uint32_t y = target.second;
  if (!valid_pos(target))
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
  if (!reachable(pos)) {
    return false;
  }
  if (filled[x][y]) {
    return false;
  }

  // Pick up a item
  switch(board[pos.first][pos.second]) {
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
    break;
  }

  filled[x][y] = true;

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

std::vector<Position> AI::get_absolute_manipulator_positions() {

  Position self = get_pos();
  std::vector<Position> ret;

  for (Position&p : worker.manipulator_range) {
    Position mani;
    switch (get_dir()) {
      case Direction::Right:
        mani = { self.first + p.first, self.second + p.second };
        break;
      case Direction::Up:
        mani = { self.first - p.second, self.second + p.first };
        break;
      case Direction::Left:
        mani = { self.first - p.first, self.second - p.second };
        break;
      case Direction::Down:
        mani = { self.first + p.second, self.second - p.first };
        break;
      default:
        assert(false);
        break;
    }
    ret.push_back(mani);
  }
  return ret;
}

void AI::next_turn() {
  current_time++;
  worker.duration_drill = std::max(0u, worker.duration_drill-1);
  worker.duration_fast = std::max(0u, worker.duration_fast-1);
}

bool AI::move(const Direction &dir) {
  const int dx[] = {0,1, 0,-1};
  const int dy[] = {1,0,-1, 0};
  int idx = static_cast<uint32_t>(dir);
  Position next_pos(worker.current_pos.first + dx[idx],
                    worker.current_pos.second + dy[idx]);

  // Check validity
  if (!valid_pos(next_pos))
    return false;

  if (board[next_pos.first][next_pos.second] == '#' && worker.duration_drill == 0)
    return false;

  // Move
  worker.current_pos = next_pos;

  // Pick up items
  for(auto p: get_absolute_manipulator_positions()) {
    fill_cell(p);
  }

  // Update time
  next_turn();

  return true;
}

//   bool move(const Direction &dir);
//   bool use_extension(const int dx, const int dy);
//   bool use_fast_wheel();
//   bool use_drill();
// };
