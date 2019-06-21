#pragma once
#include<string>
#include<vector>
#include<map>
#include "absl/strings/string_view.h"

enum class Direction {
  Right,
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
    {1, 0},
    {1, -1},
  };

  uint32_t count_fast = 0;
  uint32_t count_drill = 0;
  uint32_t count_extension = 0;

  // Remaining duration for each drill's effect.
  static const uint32_t FAST_DURATION_MAX = 50;
  static const uint32_t DRILL_DURATION_MAX = 30;
  uint32_t drill_duration = 0;
  uint32_t fast_duration = 0;
};

class AI {

  static const int32_t H_MAX = 1000;
  uint32_t current_time = 0;

  bool fill_cell(Position);

  Worker worker;

public:

  bool reachable(Position);

  AI();

  uint32_t h, w;
  std::vector<std::string> board;
  std::vector<std::vector<bool>> filled;
  uint32_t get_time();
  Position get_pos();
  Direction get_dir();
  std::vector<Position> get_range();
  uint32_t get_count_fast();
  uint32_t get_count_drill();
  uint32_t get_count_extension();

  bool move(const Direction &dir);
  bool use_extension(const int dx, const int dy);
  bool use_fast_wheel();
  bool use_drill();
  void turn_CW();
  void turn_CCW();
};
