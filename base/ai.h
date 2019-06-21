#pragma once
#include<string>
#include<vector>
#include<map>
#include "absl/strings/string_view.h"

enum class Direction {
  Right = 0,
  Down,
  Left,
  Up,
};

typedef std::pair<int32_t, int32_t> Position;

class Worker {

public:

  Worker() = default;
  Worker(Position);

  Direction current_dir = Direction::Right;
  Position current_pos;

  // relative position when Right
  std::vector<Position> manipulator_range = {
    {0, 0},
    {1, 1},
    {0, 1},
    {-1, 1},
  };

  uint32_t count_fast = 0;
  uint32_t count_drill = 0;
  uint32_t count_extension = 0;

  // Remaining duration for each drill's effect.
  static const uint32_t DURATION_FAST_MAX = 50;
  static const uint32_t DURATION_DRILL_MAX = 30;
  uint32_t duration_drill = 0;
  uint32_t duration_fast = 0;
};

class AI {
  static const int32_t H_MAX = 1000;
  uint32_t current_time = 0;
  uint32_t height = 0;
  uint32_t width = 0;

  uint32_t filled_count = 0;

  bool fill_cell(Position);

  Worker worker;

  void next_turn();
  bool valid_pos(Position);
  bool move_body(const Direction &dir);

  std::vector<std::string> executed_cmds;

public:

  AI();

  std::vector<std::string> board;
  std::vector<std::vector<bool>> filled;

  uint32_t get_height();
  uint32_t get_width();

  // Gets current time
  uint32_t get_time();
  // Gets the current position
  Position get_pos();
  // Gets the current direction
  Direction get_dir();
  // Gets the current direction
  uint32_t get_filled_count();

  // Gets the number of available boosters
  uint32_t get_count_fast();
  uint32_t get_count_drill();
  uint32_t get_count_extension();

  // Gets the all (possible) positions of manipulators.
  // It may be in invalid postion. (e.g. out of the map, unreachable postion)
  std::vector<Position> get_absolute_manipulator_positions();

  // Checks if pos is 'reachable' from the current position.
  bool reachable(Position pos);

  // Executes a command and updates the internal state.
  // If it's a invalid move, it returns false without changing internal states.
  bool move(const Direction &dir);
  bool use_extension(const int dx, const int dy);
  bool use_fast_wheel();
  bool use_drill();
  void turn_CW();  // Turn commands always succeed
  void turn_CCW();

  // Checks if get_filled_count() == Height * Width
  bool is_finished();
  // Outputs executed commands to stdout
  void write_commands();
};
