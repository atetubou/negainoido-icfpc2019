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
  return '!';
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

bool AI::reachable(Position target, const int id) {
  int x = target.first;
  int y = target.second;
  if (!valid_pos(target))
    return false;

  if (board[x][y] == '#')
    return false;

  auto worker_pos = get_pos(id);
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

  workers.clear();
  workers.push_back(Worker(worker_pos));

  for (auto &p : workers[0].manipulator_range) {
    auto x = workers[0].current_pos.first + p.first;
    auto y = workers[0].current_pos.second + p.second;
    fill_cell(std::make_pair(x, y), 0);
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

bool AI::fill_cell(Position pos, const int id) {
  int x = pos.first;
  int y = pos.second;
  if (!reachable(pos, id)) {
    return false;
  }

  if (!filled[x][y]) {
    filled_count++;
  }
  filled[x][y] = true;
  if (pos != workers[id].current_pos)
    return true;

  // Pick up a booster if pos == worker's position
  switch(board[x][y]) {
    case 'B':
      workers[id].count_extension += 1;
      break;
    case 'F':
      workers[id].count_fast += 1;
      break;
    case 'L':
      workers[id].count_drill += 1;
      break;
    default:
      break;
  }
  board[x][y] = '.';

  return true;
}

int AI::get_height() { return height; }
int AI::get_width() { return width; }

int AI::get_block_count() const { return block_count; }

int AI::get_time() { return current_time; }

Position AI::get_pos(const int id) const { return workers[id].current_pos; }

Direction AI::get_dir(const int id) { return workers[id].current_dir; }

int AI::get_filled_count() { return filled_count; }

int AI::get_count_fast(const int id) { return workers[id].count_fast; }
int AI::get_count_drill(const int id) { return workers[id].count_drill; }
int AI::get_count_extension(const int id) { return workers[id].count_extension; }

int AI::get_duration_fast(const int id) { return workers[id].duration_fast; }
int AI::get_duration_drill(const int id) { return workers[id].duration_drill; }

Position AI::get_neighbor(const Direction &dir, const int id) {
  const int dx[] = {0,1, 0,-1};
  const int dy[] = {1,0,-1, 0};
  int idx = static_cast<int>(dir);

  return Position(workers[id].current_pos.first + dx[idx],
                  workers[id].current_pos.second + dy[idx]);
}

void AI::turn_CW(const int id) {
  workers[id].current_dir =
    static_cast<Direction>( ( static_cast<int>(workers[id].current_dir) + 1 ) % 4 );
  executed_cmds.push_back("E");
  for(auto p: get_absolute_manipulator_positions()) {
    fill_cell(p);
  }

  next_turn();
}

void AI::turn_CCW(const int id) {
  workers[id].current_dir =
    static_cast<Direction>( ( static_cast<int>(workers[id].current_dir) - 1 ) % 4 );
  executed_cmds.push_back("Q");
  for(auto p: get_absolute_manipulator_positions()) {
    fill_cell(p);
  }

  next_turn();
}

std::vector<Position> AI::get_absolute_manipulator_positions(const int id) {

  Position self = get_pos(id);
  std::vector<Position> ret;

  for (Position&p : workers[id].manipulator_range) {
    Position mani = rotate(p, get_dir(id));
    mani.first += self.first;
    mani.second += self.second;
    ret.push_back(mani);
  }
  return ret;
}

void AI::next_turn() {
  current_time++;

  for (auto &worker: workers) {
    if (worker.duration_drill > 0)
      worker.duration_drill--;
    if (worker.duration_fast > 0)
      worker.duration_fast--;
  }
}

bool AI::try_move(const Direction &dir, const int id) {
  Position next_pos = get_neighbor(dir, id);

  // Check validity
  if (!valid_pos(next_pos))
    return false;

  if (board[next_pos.first][next_pos.second] == '#' && workers[id].duration_drill == 0)
    return false;

  return true;
}

bool AI::move_body(const Direction &dir, const int id) {
  // Checks if this is a valid move
  if(!try_move(dir, id))
    return false;

  auto pos = get_neighbor(dir);
  // Move
  workers[id].current_pos = pos;

  // Check drill
  if (board[pos.first][pos.second] == '#' &&
      0 < workers[id].duration_drill) {
    board[pos.first][pos.second] = '.';
    block_count--;
  }

  // Pick up items
  for(auto p: get_absolute_manipulator_positions(id)) {
    fill_cell(p, id);
  }

  return true;
}

bool AI::move(const Direction &dir, const int id) {
  if (!move_body(dir, id))
    return false;

  // When FAST booster is enabled, move twice
  if (workers[id].duration_fast > 0) {
    move_body(dir, id);
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

bool AI::use_fast_wheel(const int id) {
  if (workers[id].count_fast == 0)
    return false;

  workers[id].duration_fast = Worker::DURATION_FAST_MAX;
  workers[id].count_fast--;
  executed_cmds.push_back("F");

  next_turn();
  return true;
}

bool AI::use_drill(const int id) {
  if (workers[id].count_drill == 0)
    return false;

  workers[id].duration_drill = Worker::DURATION_DRILL_MAX;
  workers[id].count_drill--;
  executed_cmds.push_back("L");

  next_turn();
  return true;
}

bool AI::use_extension(const int dx, const int dy, const int id) {
  if (workers[id].count_extension == 0)
    return false;

  for (auto m: workers[id].manipulator_range) {
    if (Position(dx, dy) == m) {
      return false;
    }
  }

  bool can_use = false;

  for(auto m: workers[id].manipulator_range) {
    if (std::abs(m.first - dx) + std::abs(m.second - dy) == 1) {
      can_use = true;
    }
  }

  if(!can_use)
    return false;

  workers[id].manipulator_range.push_back( rotate_reverse({dx, dy}, get_dir(id)) );
  workers[id].count_extension--;

  // Convert coordinate: (dx, dy) -> (dy, -dx)
  auto cmd = "B(" + std::to_string(dy) + "," + std::to_string(-dx) + ")";
  executed_cmds.push_back(cmd);

  next_turn();
  return true;
}


void AI::dump_state() {
  std::cerr << "time: " << current_time << ", filled_count: " << filled_count << std::endl;

  std::set<Position> ws;

  for (uint32_t i = 0; i < workers.size(); i++) {
    auto p = get_pos(i);
    ws.insert(p);

    std::cerr << "pos[" << i << "]: ("
              << p.first << ", " << p.second << ")"
              << std::endl;
    std::cerr << "duration: drill=" << workers[i].duration_drill
              << ", fast: " << workers[i].duration_fast << std::endl;
  }

  for(int i = 0; i < height; ++i) {
    for(int j = 0; j < width; ++j) {
      if (ws.find(std::make_pair(i, j)) != ws.end()) {
        std::cerr << "W";
      } else if(filled[i][j]) {
        std::cerr << "V";
      } else {
        std::cerr << board[i][j];
      }
    }
    std::cerr << std::endl;;
  }
}
