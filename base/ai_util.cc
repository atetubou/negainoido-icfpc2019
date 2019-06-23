/*
 * This file contains utility libraries for ai.h.
 */

#include <iostream>
#include <fstream>
#include <cassert>
#include <queue>
#include "base/base.h"
#include "base/ai.h"
#include "base/geometry.h"
#include "absl/strings/str_split.h"
#include <glog/logging.h>

typedef std::pair<Position, Direction> State;

std::vector<Position> get_absolute_pos(const std::vector<Position>& range,
                                       State state, int id) {
  Position pos = state.first;
  Direction dir = state.second;

  std::vector<Position> ret;

  for (auto& p : range) {
    Position mani = rotate(p, dir);
    mani.first += pos.first;
    mani.second += pos.second;
    ret.push_back(mani);
  }
  return ret;
}

std::vector<Command> AI::shortest_filling_commands(Position dst, const int id) const {
  if (!valid_pos(dst))
    LOG(FATAL) << "invalid position: (" << dst.first << ", " << dst.second;

  if (filled[dst.first][dst.second])
    return std::vector<Command>();

  const State init_state = std::make_pair(get_pos(id), get_dir(id));

  std::queue<State> que;
  std::map<State, State> prev;
  std::map<State, Command> prev_cmd;

  que.push(init_state);

  bool done = false;
  State final_state ;
  while (!que.empty()) {
    State state = que.front();
    que.pop();

    // Check if dst is filled by manipulator
    for (auto &p : get_absolute_pos(workers[id].manipulator_range,
                                                state, id)) {
      if (p == dst) {
        done = true;
        final_state = state;

        goto done;
      }
    }


    static const int dx[] = {0, 1, 0, -1};
    static const int dy[] = {1, 0, -1, 0};
    // Push next states
    // Move
    for (int i = 0; i < 4; i++) {
      auto pos = Position(state.first.first + dx[i], state.first.second + dy[i]);
      auto dir = state.second;

      State nstate = std::make_pair(pos, dir);

      if (!valid_pos(pos))
        continue;

      if (board[pos.first][pos.second] == '#')
        continue;


      if (prev.find(nstate) == prev.end()) {
        que.push(nstate);
        prev[nstate] = state;
        prev_cmd[nstate] = {CmdType::Move, static_cast<Direction>(i)};

        que.push(nstate);
      }
    }

    // CW
    {
      auto pos = state.first;
      auto dir = static_cast<Direction>((static_cast<int>(state.second) + 1) % 4);
      State nstate = std::make_pair(pos, dir);

      if (prev.find(nstate) == prev.end()) {
        que.push(nstate);
        prev[nstate] = state;
        prev_cmd[nstate] = {CmdType::TurnCW};

        que.push(nstate);
      }
    }

    // CCW
    {
      auto pos = state.first;
      auto dir = static_cast<Direction>((static_cast<int>(state.second) + 3) % 4);
      State nstate = std::make_pair(pos, dir);

      if (prev.find(nstate) == prev.end()) {
        que.push(nstate);
        prev[nstate] = state;
        prev_cmd[nstate] = {CmdType::TurnCCW};

        que.push(nstate);
      }
    }
  }

  LOG_IF(FATAL, !done) << "Unreachable!: from ("
                       << get_pos(id).first << ", " << get_pos(id).second << ") to ("
                       << dst.first << ", " << dst.second;

 done:

  // Construct the path by backtracking prev and prev_cmd.
  std::vector<State> states;
  std::vector<Command> cmds;

  State cur_state = final_state;
  states.push_back(cur_state);

  while (cur_state != init_state) {
    cmds.push_back(prev_cmd[cur_state]);

    cur_state = prev[cur_state];
  }

  reverse(cmds.begin(), cmds.end());

  return cmds;
}
