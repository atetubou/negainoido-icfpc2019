#pragma once

#include<string>
#include<vector>
#include<map>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

enum class Direction {
  Right = 0,
  Down,
  Left,
  Up,
};

char direction_to_char(Direction d);
absl::optional<Direction> char_to_direction(char c);

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

  int count_fast = 0;
  int count_drill = 0;
  int count_extension = 0;

  // Remaining duration for each drill's effect.
  static const int DURATION_FAST_MAX = 50;
  static const int DURATION_DRILL_MAX = 30;
  int duration_drill = 0;
  int duration_fast = 0;
};

class AI {
  static const int32_t H_MAX = 1000;
  int current_time = 0;
  int height = 0;
  int width = 0;

  int filled_count = 0;
  int block_count = 0;

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

  int get_height();
  int get_width();

  // Gets current time
  int get_time();
  // Gets the current position
  Position get_pos();
  // Gets the current direction
  Direction get_dir();
  // Gets the current direction
  int get_filled_count();

  // Gets the number of available boosters
  int get_count_fast();
  int get_count_drill();
  int get_count_extension();

  // Returns the position that is next to the current position.
  Position get_neighbor(const Direction &dir);

  // Gets the all (possible) positions of manipulators.
  // It may be in invalid postion. (e.g. out of the map, unreachable postion)
  std::vector<Position> get_absolute_manipulator_positions();

  // Checks if pos is 'reachable' from the current position.
  bool reachable(Position pos);

  // Checks if the robot can move to the direction.
  // This doesn't change any internal state.
  bool try_move(const Direction &dir);

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
  void print_commands();


  // Prints AI's state for debugging.
  void dump_state();
};
