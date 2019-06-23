#include <iostream>
#include <fstream>
#include <cassert>
#include "base/base.h"
#include "base/ai.h"
#include "base/geometry.h"
#include "absl/strings/str_split.h"
#include <glog/logging.h>

Position dir2vec(const Direction &dir) {
  static const int dx[] = {0, 1, 0, -1};
  static const int dy[] = {1, 0, -1, 0};
  int idx = static_cast<int>(dir);

  return Position(dx[idx], dy[idx]);
}

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
  return Position(-1000000, -1000000);
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

bool AI::valid_pos(Position target) const {
  int x = target.first;
  int y = target.second;
  return 0 <= x && x < height && 0 <= y && y < width;
}

bool AI::reachable(Position target, const int id) const {
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
      } else if (board[i][j] == 'X') {
	spawn_points.emplace_back(i, j);
      } else if (board[i][j] == 'C') {
	cloning_points.emplace_back(i, j);
      }
    }
  }
  filled.resize(height);
  for (int i = 0; i < height; ++i) filled[i].resize(width);

  workers.clear();
  workers.push_back(Worker(worker_pos));

  executed_cmds.resize(1);

  for (auto &p : workers[0].manipulator_range) {
    auto x = workers[0].current_pos.first + p.first;
    auto y = workers[0].current_pos.second + p.second;
    fill_cell(std::make_pair(x, y), 0);
  }

  pickup_booster(0);
}

bool AI::init_turn(const int id) {
  LOG_IF(FATAL, expect_worker_id != id)
    << "command for the worker " << expect_worker_id
    << "must be called but the ID was" << id;

  // Pick up a booster.
  // See Appendix in clone document
  pickup_booster(id);

  return !is_finished();
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

void AI::pickup_booster(const int id) {
  auto p = get_pos(id);


  switch(board[p.first][p.second]) {
  case 'B':
    count_extension += 1;
    break;
  case 'F':
    count_fast += 1;
    break;
  case 'L':
    count_drill += 1;
    break;
  case 'C':
    count_clone += 1;
    break;
  case 'R':
    count_teleport += 1;
    break;
  default:
    break;
  }
  board[p.first][p.second] = '.';

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

  return true;
}

int AI::get_height() { return height; }
int AI::get_width() { return width; }

int AI::get_block_count() const { return block_count; }

int AI::get_time() { return current_time; }

Position AI::get_pos(const int id) const { return workers[id].current_pos; }

Direction AI::get_dir(const int id) { return workers[id].current_dir; }

int AI::get_filled_count() { return filled_count; }

int AI::get_count_fast() { return count_fast; }
int AI::get_count_drill() { return count_drill; }
int AI::get_count_extension() { return count_extension; }
int AI::get_count_clone() { return count_clone; }
int AI::get_count_teleport() { return count_teleport; }

int AI::get_count_workers() { return workers.size(); }

int AI::get_duration_fast(const int id) { return workers[id].duration_fast; }
int AI::get_duration_drill(const int id) { return workers[id].duration_drill; }

Position AI::get_neighbor(const Direction &dir, const int id) {
  const int dx[] = {0,1, 0,-1};
  const int dy[] = {1,0,-1, 0};
  int idx = static_cast<int>(dir);

  return Position(workers[id].current_pos.first + dx[idx],
                  workers[id].current_pos.second + dy[idx]);
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

std::vector<Position> AI::rotated_manipulator_positions(bool is_cw, const int id) {
  Position self = get_pos(id);
  std::vector<Position> ret;

  for (Position&p : workers[id].manipulator_range) {
    Direction cur_dir = get_dir(id);
    int ndir = (static_cast<int>(cur_dir) + (is_cw ? 1 : 3)) % 4;
    Direction next_dir = static_cast<Direction>(ndir);

    Position mani = rotate(p, next_dir);
    mani.first += self.first;
    mani.second += self.second;

    if(reachable(mani, id))
      ret.push_back(mani);
  }
  return ret;
}

std::vector<Position> AI::moved_manipulator_positions(const Direction& d, const int id) const {
  Position self = get_pos(id);
  std::vector<Position> ret;

  const auto v = dir2vec(d);

  for (const Position &p : workers[id].manipulator_range) {
    const Position mani = {self.first + p.first + v.first,
			   self.second + p.second + v.second};
    
    if(reachable(mani, id)) {
      ret.push_back(mani);
    }
  }

  return ret;
}

void AI::next_turn(const int id) {
  if (id == (int)workers.size()-1)
    current_time++;

  expect_worker_id += 1;
  expect_worker_id %= workers.size();

  if (workers[id].duration_drill > 0)
    workers[id].duration_drill--;
  if (workers[id].duration_fast > 0)
    workers[id].duration_fast--;
}

void AI::push_command(Command cmd, const int id) {
  LOG_IF(FATAL, id >= (int)executed_cmds.size()) << id << " is out of range " << workers.size();

  executed_cmds[id].push_back(cmd);
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
  if(!init_turn(id))
    return false;

  if (!move_body(dir, id))
    return false;

  // When FAST booster is enabled, move twice
  if (workers[id].duration_fast > 0) {
    move_body(dir, id);
  }

  Command cmd = {CmdType::Move, dir};
  push_command(cmd, id);

  // Update time
  next_turn(id);

  return true;
}

bool AI::turn_CW(const int id) {
  if(!init_turn(id))
    return false;

  workers[id].current_dir =
    static_cast<Direction>( ( static_cast<int>(workers[id].current_dir) + 1 ) % 4 );

  for(auto p: get_absolute_manipulator_positions()) {
    fill_cell(p);
  }

  Command cmd = {CmdType::TurnCW};
  push_command(cmd, id);

  next_turn(id);

  return true;
}

bool AI::turn_CCW(const int id) {
  if(!init_turn(id))
    return false;

  workers[id].current_dir =
    static_cast<Direction>( ( static_cast<int>(workers[id].current_dir) + 3 ) % 4 );

  for(auto p: get_absolute_manipulator_positions()) {
    fill_cell(p);
  }

  Command cmd = {CmdType::TurnCCW};
  push_command(cmd, id);

  next_turn(id);

  return true;
}

bool AI::use_fast_wheel(const int id) {
  if(!init_turn(id))
    return false;

  if (count_fast == 0)
    return false;

  workers[id].duration_fast = Worker::DURATION_FAST_MAX;
  count_fast--;

  Command cmd = {CmdType::UseFastWheel};
  push_command(cmd, id);

  next_turn(id);
  return true;
}

bool AI::use_drill(const int id) {
  if(!init_turn(id))
    return false;
  if (count_drill == 0)
    return false;

  workers[id].duration_drill = Worker::DURATION_DRILL_MAX;
  count_drill--;

  Command cmd = {CmdType::UseDrill};
  push_command(cmd, id);

  next_turn(id);

  return true;
}

bool AI::use_extension(const int dx, const int dy, const int id) {
  if(!init_turn(id))
    return false;

  if (count_extension == 0)
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
  count_extension--;

  // Fill a cell visited by a new manipulator.
  // Update all manipulator's cell by get_absolute_manipulator_positions() because I'm lazy.
  for(auto p: get_absolute_manipulator_positions(id)) {
    fill_cell(p, id);
  }

  Command cmd = {CmdType::UseExtension, Direction::Right /* dymmy */, dx, dy};
  push_command(cmd, id);

  next_turn(id);

  return true;
}

bool AI::use_clone(const int id) {
  if(!init_turn(id))
    return false;
  if (count_clone == 0)
    return false;

  auto p = get_pos(id);
  if (board[p.first][p.second] != 'X')
    return false;

  count_clone--;

  // Create a child worker
  Worker child(p);
  workers.push_back(child);

  executed_cmds.push_back(std::vector<Command>());

  Command cmd = {CmdType::UseClone};
  push_command(cmd, id);

  next_turn(id);
  return true;
}

bool AI::install_beacon(const int id) {
  if(!init_turn(id))
    return false;

  auto p = get_pos(id);
  if (board[p.first][p.second] == 'X' ||
      board[p.first][p.second] == 'b') {
    return false;
  }

  if (count_teleport == 0)
    return false;

  count_teleport--;
  board[p.first][p.second] = 'b';
  beacon_pos.insert(p);

  push_command({CmdType::InstallBeacon}, id);

  next_turn(id);
  return true;
}

bool AI::jump_to_beacon(Position dst, const int id) {
  if(!init_turn(id))
    return false;

  if (beacon_pos.find(dst) == beacon_pos.end()) {
    return false;
  }

  workers[id].current_pos = dst;

  Command com = {CmdType::JumpToBeacon, Direction::Right /* dummy*/, dst.first, dst.second};
  push_command(com, id);

  next_turn(id);
  return true;
}

bool AI::do_command(Command cmd, const int id) {
  switch (cmd.type) {
  case CmdType::Move:
    return move(cmd.dir, id);
  case CmdType::TurnCW:
    return turn_CW(id);
  case CmdType::TurnCCW:
    return turn_CCW(id);
  case CmdType::UseExtension:
    return use_extension(cmd.x, cmd.y, id);
  case CmdType::UseFastWheel:
    return use_fast_wheel(id);
  case CmdType::UseDrill:
    return use_drill(id);
  case CmdType::UseClone:
    return use_clone(id);
  case CmdType::InstallBeacon:
    return install_beacon(id);
  case CmdType::JumpToBeacon:
    return jump_to_beacon(Position(cmd.x, cmd.y), id);
  }

  LOG(FATAL) << "BUG?: unknown command name: " << static_cast<int>(cmd.type);
}

bool AI::is_finished() {
  return get_filled_count() + block_count == height * width;
}

void print_command(struct Command cmd, int height) {
  std::string s = "";

  switch (cmd.type) {
  case CmdType::Move:
    switch (cmd.dir) {
    case Direction::Right:
      s = "D";
      break;
    case Direction::Down:
      s = "S";
      break;
    case Direction::Left:
      s = "A";
      break;
    case Direction::Up:
      s = "W";
      break;
    }
    break;
  case CmdType::TurnCW:
    s = "E";
    break;
  case CmdType::TurnCCW:
    s = "Q";
    break;
  case CmdType::UseExtension:
    // Convert coordinate: (dx, dy) -> (dy, -dx)
    s = "B(" + std::to_string(cmd.y) + "," + std::to_string(-cmd.x) + ")";
    break;
  case CmdType::UseFastWheel:
    s = "F";
    break;
  case CmdType::UseDrill:
    s = "L";
    break;
  case CmdType::UseClone:
    s = "C";
    break;
  case CmdType::InstallBeacon:
    // Convert coordinate: (dx, dy) -> (dy, -dx)
    s = "R";
    break;
  case CmdType::JumpToBeacon:
    s = "T(" + std::to_string(cmd.y) + "," + std::to_string(height - cmd.x) + ")";
    break;
  default:
    LOG(FATAL) << "BUG?: unknown command name: " << static_cast<int>(cmd.type);
  }

  LOG_IF(FATAL, s.size() == 0) << "BUG: empty command: " << static_cast<int>(cmd.type);

  std::cout << s;
}

void AI::print_commands() {
  for(int i = 0; i < (int) executed_cmds.size(); ++i) {
    if (i != 0)
      std::cout << "#";

    for (auto cmd: executed_cmds[i]) {
      print_command(cmd, height);
    }
  }
  std::cout << std::endl;
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
