
/*
The algorithm is same with tikuta_solver's one.

example usage:
$ bazel run //sample_ai:sample_ai < part-1-initial/prob-001.in
WWDSAWDDSAWDDS
*/


#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"

#include "base/ai.h"

using namespace std;

int expect_char(const string &commands, int pos, const char c) {
  uint32_t i = pos;
  while(i < commands.size()) {
    if (commands[i] == c)
      return i;
    i++;
  }
  LOG(FATAL) << "'" << c << "' is expected after position " << pos
             << ", but not found";
  return -1;
}


pair<Position, int> expect_pair(AI &ai, const string &commands, int pos) {
  LOG_IF(FATAL, commands[pos] != '(') << "expeced '(' but '" << commands[pos] << "'";

  int pos1 = expect_char(commands, pos, ',');
  int pos2 = expect_char(commands, pos1, ')');

  int x, y;
  try {
    x = stoi(commands.substr(pos  + 1, pos1 - pos  - 1));
    y = stoi(commands.substr(pos1 + 1, pos2 - pos1 - 1));
  } catch (const invalid_argument& e) {
    LOG(FATAL) << "position " << pos
               << "pair was expeced but '" << commands.substr(pos, pos2 - pos + 1) << "'";
  }

  return make_pair(Position(x, y), pos2);
}

int check_cmd(AI &ai, const string &commands, int pos) {
  switch(commands[pos]) {
  case 'W':
    LOG_IF(FATAL, !ai.move(Direction::Up))
      << "failed to 'move up' at " << pos;
    return pos+1;
  case 'S':
    LOG_IF(FATAL, !ai.move(Direction::Down))
      << "failed to 'move down' at " << pos;
    return pos+1;
  case 'A':
    LOG_IF(FATAL, !ai.move(Direction::Left))
      << "failed to 'move left' at " << pos;
    return pos+1;
  case 'D':
    LOG_IF(FATAL, !ai.move(Direction::Right))
      << "failed to 'move right' at " << pos;
    return pos+1;
  case 'E':
    ai.turn_CW();
    return pos+1;
  case 'Q':
    ai.turn_CCW();
    return pos+1;
  case 'B': {
    auto r = expect_pair(ai, commands, pos);
    auto p = r.first;
    int np = r.second;
    LOG_IF(FATAL, !ai.use_extension(p.first, p.second))
      << "failed to 'attach a new manipulator (" << p.first << "," << p.second << ")' at " << pos;
    return np + 1;
  }
  case 'F':
    LOG_IF(FATAL, !ai.use_fast_wheel())
      << "failed to 'use fast wheel' at " << pos;
    return pos+1;
  case 'L':
    LOG_IF(FATAL, !ai.use_drill())
      << "failed to 'use drill' at " << pos;
    return pos+1;
  default:
    LOG(FATAL) << "Unknown character: '" << commands[pos]
               << "' at position " << pos;
  }
}

DEFINE_string(in, "", ".in file name");
DEFINE_string(sol, "", ".sol file name");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  // AI's constructor accepts a input file from stdin.
  AI ai(FLAGS_in.c_str());

  ifstream t(FLAGS_sol.c_str());
  string file_content((istreambuf_iterator<char>(t)),
                           istreambuf_iterator<char>());

  int cnt;
  if ((cnt = count(file_content.begin(), file_content.end(), '\n')) > 1) {
    LOG(FATAL) << "file " << FLAGS_sol << " has multiple lines: " << cnt;
  }
  vector<string> cmd_vec = absl::StrSplit(file_content, '\n');
  LOG_IF(FATAL, cmd_vec.size() > 2) << "line count of " << FLAGS_sol << " must be 1 but " << cmd_vec.size();
  string commands = cmd_vec[0];

  uint32_t pos = 0;

  while(pos < commands.size()) {
    pos = check_cmd(ai, commands, pos);
  }

  if(!ai.is_finished()) {
    ai.dump_state();
    LOG(FATAL) << "AI terminated before it visits all cells";
  }

  cout << ai.get_time() << endl;
}
