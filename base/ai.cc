#include <iostream>
#include <fstream>
#include <cassert>
#include "base/base.h"
#include "base/ai.h"
#include "base/geometry.h"
#include "absl/strings/str_split.h"
#include <glog/logging.h>

char direction_to_char(Direction d) {
  switch (d) {
  case Direction::Right:
    return 'D';
  case Direction::Left:
    return 'A';
  case Direction::Up:
    return 'W';
  case Direction::Down:
    return 'S';
  }

  LOG(FATAL) << "Invalid direction " << static_cast<int>(d);
}

// relative p when Right => ??? when dir
Position rotate(Position p, Direction d) {
  switch (d) {
  case Direction::Right:
    return { p.first, p.second };
  case Direction::Up:
    return { -p.second, p.first };
  case Direction::Left:
    return { -p.first, -p.second };
  case Direction::Down:
    return { p.second, -p.first };
  }
  LOG(FATAL) << "Invalid direction " << static_cast<int>(d);
}

// relative p when dir => ??? when Right
Position rotate_reverse(Position p, Direction dir) {
  Position q = rotate(p, dir);
  q = rotate(q, dir);
  q = rotate(q, dir);
  return q;
}

absl::optional<Direction> char_to_direction(char c) {
  switch(c) {
  case 'D':
    return Direction::Right;
  case 'A':
    return Direction::Left;
  case 'W':
    return Direction::Up;
  case 'S':
    return Direction::Down;
  };

  return absl::nullopt;
}

Worker::Worker(Position pos) {
  current_pos = pos;
}

bool AI::valid_pos(Position target) {
  int x = target.first;
  int y = target.second;
  return 0 <= x && x < height && 0 <= y && y < width;
}

bool AI::reachable(Position target) {
  int x = target.first;
  int y = target.second;
  if (!valid_pos(target))
    return false;

  if (board[x][y] == '#')
    return false;

  auto worker_pos = get_pos();
  int wx = worker_pos.first;
  int wy = worker_pos.second;
  for (int cx = std::min(x, wx); cx <= std::max(x, wx); cx++) {
    for (int cy = std::min(y, wy); cy <= std::max(y, wy); cy++) {
      if (board[cx][cy] != '#')
        continue;

      Line l({wx + 0.5, wy + 0.5}, {x + 0.5, y + 0.5});

      Point cell(cx+0.5, cy+0.5);
      if (intersectSP(l, cell))
        return false;

      for(int i = 0; i < 4; i++) {
        const int dx[] = {cx, cx+1, cx+1, cx};
        const int dy[] = {cy, cy, cy+1, cy+1};
        Line edge({(double)dx[i], (double)dy[i]}, {(double)dx[(i+1)%4], (double)dy[(i+1)%4]});

        if (intersectSS(l, edge))
          return false;
      }
    }
  }

  return true;
}

void AI::initialize() {
  std::cin >> height >> width;
  std::string buf;
  while (std::cin >> buf) {
    board.push_back(buf);
  }

  Position worker_pos = std::make_pair(0, 0);
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      if (board[i][j] == 'W') {
        worker_pos = std::make_pair(i, j);
      } else if (board[i][j] == '#') {
        block_count++;
      }
    }
  }
  filled.resize(height);
  for (int i = 0; i < height; ++i) filled[i].resize(width);

  worker = Worker(worker_pos);

  for (auto&p : worker.manipulator_range) {
    auto x = worker.current_pos.first + p.first;
    auto y = worker.current_pos.second + p.second;
    fill_cell(std::make_pair(x, y));
  }
}

AI::AI() {
  initialize();
}

AI::AI(const std::string filename) {
  std::ifstream in(filename.c_str());
  LOG_IF(FATAL, !in) <<
    "failed to open " << filename;

  std::cin.rdbuf(in.rdbuf());

  initialize();
}

bool AI::fill_cell(Position pos) {
  int x = pos.first;
  int y = pos.second;
  if (!reachable(pos)) {
    return false;
  }
  if (filled[x][y]) {
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
      break;
  }

  filled[x][y] = true;
  filled_count++;

  return true;
}

int AI::get_height() { return height; }
int AI::get_width() { return width; }

int AI::get_time() { return current_time; }

Position AI::get_pos() { return worker.current_pos; }

Direction AI::get_dir() { return worker.current_dir; }

int AI::get_filled_count() { return filled_count; }

int AI::get_count_fast() { return worker.count_fast; }
int AI::get_count_drill() { return worker.count_drill; }
int AI::get_count_extension() { return worker.count_extension; }

Position AI::get_neighbor(const Direction &dir) {
  const int dx[] = {0,1, 0,-1};
  const int dy[] = {1,0,-1, 0};
  int idx = static_cast<int>(dir);

  return Position(worker.current_pos.first + dx[idx],
                  worker.current_pos.second + dy[idx]);
}

void AI::turn_CW() {
  worker.current_dir =
    static_cast<Direction>( ( static_cast<int>(worker.current_dir) + 1 ) % 4 );
  executed_cmds.push_back("E");
}

void AI::turn_CCW() {
  worker.current_dir =
    static_cast<Direction>( ( static_cast<int>(worker.current_dir) - 1 ) % 4 );
  executed_cmds.push_back("Q");
}

std::vector<Position> AI::get_absolute_manipulator_positions() {

  Position self = get_pos();
  std::vector<Position> ret;

  for (Position&p : worker.manipulator_range) {
    Position mani = rotate(p, get_dir());
    mani.first += self.first;
    mani.second += self.second;
    ret.push_back(mani);
  }
  return ret;
}

void AI::next_turn() {
  current_time++;
  if (worker.duration_drill > 0)
    worker.duration_drill--;
  if (worker.duration_fast > 0)
    worker.duration_fast--;
}

bool AI::try_move(const Direction &dir) {
  Position next_pos = get_neighbor(dir);

  // Check validity
  if (!valid_pos(next_pos))
    return false;

  if (board[next_pos.first][next_pos.second] == '#' && worker.duration_drill == 0)
    return false;

  return true;
}

bool AI::move_body(const Direction &dir) {
  // Checks if this is a valid move
  if(!try_move(dir))
    return false;

  // Move
  worker.current_pos = get_neighbor(dir);

  // Pick up items
  for(auto p: get_absolute_manipulator_positions()) {
    fill_cell(p);
  }

  return true;
}

bool AI::move(const Direction &dir) {
  if (!move_body(dir))
    return false;

  // When FAST booster is enabled, move twice
  if (worker.duration_fast > 0) {
    move_body(dir);
  }

  // Push commandx
  const std::string dir2cmd[4] = {
    "D", "S", "A", "W"
  };
  executed_cmds.push_back(dir2cmd[static_cast<int>(dir)]);

  // Update time
  next_turn();

  return true;
}

bool AI::is_finished() {
  return get_filled_count() + block_count == height * width;
}

void AI::print_commands() {
  for(auto c: executed_cmds) {
    std::cout << c;
  }
  std::cout << std::endl;
}

bool AI::use_fast_wheel() {
  if (worker.count_fast == 0)
    return false;

  worker.duration_fast = Worker::DURATION_FAST_MAX;
  worker.count_fast--;
  executed_cmds.push_back("F");

  return true;
}

bool AI::use_drill() {
  if (worker.count_drill == 0)
    return false;

  worker.duration_drill = Worker::DURATION_DRILL_MAX;
  worker.count_drill--;
  executed_cmds.push_back("L");

  return true;
}

bool AI::use_extension(const int dx, const int dy) {
  if (worker.count_extension == 0)
    return false;

  bool can_use = false;
  for(auto m: worker.manipulator_range) {
    if (std::abs(m.first - dx) + std::abs(m.second - dy)) {
      can_use = true;
    }
  }

  if(!can_use)
    return false;

  worker.manipulator_range.push_back( rotate_reverse({dx, dy}, get_dir()) );
  worker.count_extension--;
  auto cmd = "B(" + std::to_string(dx) + "," + std::to_string(dx) + ")";
  executed_cmds.push_back(cmd);

  return true;
}


void AI::dump_state() {
  std::cerr << "time: " << current_time << ", filled_count: " << filled_count << std::endl;
  std::cerr << "pos: (" << worker.current_pos.first << ", " << worker.current_pos.second << ")" << std::endl;
  std::cerr << "duration: drill=" << worker.duration_drill << ", fast: " << worker.duration_fast << std::endl;
  auto p = get_pos();

  for(int i = 0; i < height; ++i) {
    for(int j = 0; j < width; ++j) {
      if(i == p.first && j == p.second) {
        std::cerr << "@";
      } else if(filled[i][j]) {
        std::cerr << "V";
      } else {
        std::cerr << board[i][j];
      }
    }
    std::cerr << std::endl;;
  }
}
