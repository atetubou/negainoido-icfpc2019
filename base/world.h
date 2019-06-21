#pragma once
#include<string>
#include<vector>
#include<map>

enum class Direction {
  Right,
  Down,
  Left,
  Up,
};

typedef std::pair<int32_t, int32_t> Position;

class Worker {

  Direction current_dir;
  uint32_t current_pos;

  std::vector<Position> manipulator_range;

  // Remaining duration for each drill's effect.
  const uint32_t FAST_DURATION_MAX = 50;
  const uint32_t DRILL_DURATION_MAX = 30;
  uint32_t drill_duration;
  uint32_t fast_duration;

};

class World {
  static const int32_t H_MAX = 1000;
  uint32_t current_time;
  std::string board[World::H_MAX];

  Worker worker;

public:
  uint32_t get_time();
  std::pair<uint32_t, uint32_t> get_pos();
  std::vector<Position> get_range();

  bool move(const Direction &dir);
  bool use_extension(const int dx, const int dy);
  bool use_fast_wheel();
  bool use_drill();
  bool turn_CW();
  bool turn_CCW();
};
