#pragma once

#include<string>
#include<vector>
#include<map>
#include<set>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

enum class Direction {
  Right = 0,
  Down,
  Left,
  Up,
};

typedef std::pair<int, int> Position;

Position dir2vec(const Direction &dir);

enum class CmdType {
  Move = 0,
  TurnCW,
  TurnCCW,
  UseExtension,
  UseFastWheel,
  UseDrill,
  UseClone,
  InstallBeacon,
  JumpToBeacon,
};

struct Command {
  CmdType type;

  // valid if type == "Move"
  Direction dir;

  // valid if type == UseExtension || type == JumpToBeacon
  int x;
  int y;
};

char direction_to_char(Direction d);
absl::optional<Direction> char_to_direction(char c);

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

  bool fill_cell(Position, const int id = 0);

  int count_fast = 0;
  int count_drill = 0;
  int count_extension = 0;
  int count_clone = 0;
  int count_teleport = 0;

  int expect_worker_id = 0;   // Rotate from 0 to (workers.size()-1)
  std::vector<Worker> workers;

  void next_turn(const int id);
  bool valid_pos(Position);
  bool move_body(const Direction &dir, const int id = 0);


  void initialize();

  void init_turn(const int id);
  void pickup_booster(const int id);

  void push_command(struct Command cmd, const int id);

public:

  AI();
  AI(const std::string filename);

  std::vector<std::string> board;
  std::vector<std::vector<bool>> filled;
  std::vector<std::vector<Command>> executed_cmds;

  // Positions of 'X'.
  std::vector<Position> spawn_points;
  // Positions of 'C'.
  std::vector<Position> cloning_points;

  std::set<Position> beacon_pos;

  int get_height();
  int get_width();

  int get_block_count() const;

  // Gets current time
  int get_time();
  // Gets the current position
  Position get_pos(const int id = 0) const;

  // Gets the current direction
  Direction get_dir(const int id = 0);
  // Gets the number of filled cells
  int get_filled_count();

  // Gets the number of available boosters
  int get_count_fast();
  int get_count_drill();
  int get_count_extension();
  int get_count_clone();
  int get_count_teleport();

  int get_count_workers();

  // Gets the unit time until each tool is consumed.
  // Returns 0 if the tool is not used now.
  int get_duration_fast(const int id = 0);
  int get_duration_drill(const int id = 0);

  // Returns the position that is next to the current position.
  Position get_neighbor(const Direction &dir, const int id = 0);

  // Gets the all (possible) positions of manipulators.
  // It may be in invalid postion. (e.g. out of the map, unreachable postion)
  std::vector<Position> get_absolute_manipulator_positions(const int id = 0);

  // Checks if pos is 'reachable' from the current position.
  bool reachable(Position pos, const int id = 0);

  // Checks if the robot can move to the direction.
  // This doesn't change any internal state.
  bool try_move(const Direction &dir, const int id = 0);

  // Executes a command and updates the internal state.
  // If it's a invalid move, it returns false without changing internal states.
  bool move(const Direction &dir, const int id = 0);
  bool use_extension(const int dx, const int dy, const int id = 0);
  void turn_CW(const int id = 0);  // Turn commands always succeed
  void turn_CCW(const int id = 0);

  bool use_fast_wheel(const int id = 0);
  bool use_drill(const int id = 0);
  bool use_clone(const int id);

  bool install_beacon(const int id);
  bool jump_to_beacon(Position dst, const int id);

  // Checks if get_filled_count() == Height * Width
  bool is_finished();
  // Outputs executed commands to stdout
  void print_commands();


  // Prints AI's state for debugging.
  void dump_state();
};
