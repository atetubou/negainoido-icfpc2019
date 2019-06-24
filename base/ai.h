#pragma once

#include<iostream>
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

std::ostream& operator<<(std::ostream& os, const Direction& d);

typedef std::pair<int, int> Position;

Position dir2vec(const Direction &dir);
char direction_to_char(Direction d);
absl::optional<Direction> char_to_direction(char c);

Position rotate(Position p, Direction d);

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
  Nop,
};

struct Command {
  CmdType type;

  // valid if type == "Move"
  Direction dir;

  // valid if type == UseExtension || type == JumpToBeacon
  int x;
  int y;
};

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

  bool fill_cell(Position, const int id);

  int count_fast = 0;
  int count_drill = 0;
  int count_extension = 0;
  int count_clone = 0;
  int count_teleport = 0;

  int next_worker_id = 0;
  int count_active_workers = 1;
  std::vector<Worker> workers;

  void next_turn(const int id);
  bool valid_pos(Position) const;
  bool move_body(const Direction &dir, const int id);


  void get_board_from_stdin();
  void initialize();

  bool init_turn(const int id);
  void pickup_booster(const int id);

  bool reachable_sub(Position src, Position pos) const;

  void push_command(struct Command cmd, const int id);

public:

  AI();
  // accepts a string representing buyfile's content
  AI(const std::string buystring);

  AI(const std::vector<std::string> &init_board, const std::vector<std::vector<bool>> &init_filled);

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
  Direction get_dir(const int id = 0) const;
  // Gets the number of filled cells
  int get_filled_count() const;

  // Gets the number of available boosters
  int get_count_fast();
  int get_count_drill();
  int get_count_extension();
  int get_count_clone();
  int get_count_teleport();

  // Gets number of workers in this turn.
  // i.e. workers.size() - |workers cloned in this turn|
  int get_count_active_workers() const;

  int get_next_worker_id() const;

  // Gets the unit time until each tool is consumed.
  // Returns 0 if the tool is not used now.
  int get_duration_fast(const int id = 0);
  int get_duration_drill(const int id = 0);

  // Returns the position that is next to the current position.
  Position get_neighbor(const Direction &dir, const int id = 0) const;

  // Gets the all (possible) positions of manipulators.
  // It may be in invalid postion. (e.g. out of the map, unreachable postion)
  std::vector<Position> get_absolute_manipulator_positions(const int id = 0) const;

  // Gets the all manipulators' positions after one rotation.
  // Unreachable/invalid positions are skipped.
  std::vector<Position> rotated_manipulator_positions(bool is_cw, const int id = 0);

  // Gets the all manipulators' positions after one move.
  // Unreachable/invalid positions are skipped.
  std::vector<Position> moved_manipulator_positions(const Direction& d, const int id = 0) const;

  // Checks if pos is 'reachable' from the current position.
  bool reachable(Position pos, const int id = 0) const;

  // Checks if the robot can move to the direction.
  // This doesn't change any internal state.
  bool try_move(const Direction &dir, const int id = 0) const;

  /*
   * Command execution
   */

  // Executes a command. This is a wrapper of 'move', 'use_extesion', etc.
  bool do_command(Command cmd, const int id = 0);

  // Executes a command and updates the internal state.
  // If it's a invalid move, it returns false without changing internal states.
  bool move(const Direction &dir, const int id = 0);
  bool use_extension(const int dx, const int dy, const int id = 0);
  bool turn_CW(const int id = 0);  // Turn commands always succeed
  bool turn_CCW(const int id = 0);

  bool use_fast_wheel(const int id = 0);
  bool use_drill(const int id = 0);
  bool use_clone(const int id = 0);

  bool install_beacon(const int id = 0);
  bool jump_to_beacon(Position dst, const int id = 0);

  bool nop(const int id = 0);

  // Checks if get_filled_count() == Height * Width
  bool is_finished() const;

  // Returns command sequence in official format
  std::string commands2str() const;

  // Outputs executed commands to stdout
  void print_commands() const;


  // Prints AI's state for debugging.
  void dump_state() const;


  /*
   * Utility
   */

  // Returns a shortest command sequence which fills dst.
  // The returned sequence only conistents of Move. TurnCW and TurnCCW.
  std::vector<Command> shortest_filling_commands(Position dst, const int id = 0);
};
