#include <iostream>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>

#include "absl/types/optional.h"

#define NOT_REACHED() assert(false)

enum State : uint8_t {
  WALL = 0,
  GARBAGE,
  CLEAR,
};

using Point = std::pair<int,int>;

//#define HIROH_DEBUG

#ifdef HIROH_DEBUG
#define print_point(p) std::cerr << #p << "=("<< p.first << ", " << p.second << ")" <<std::endl
#else
#define print_point(p)
#endif

#define MAX_L 500

int W, H;
// board[W][H]
uint8_t board[MAX_L][MAX_L];

void print_table() {
#ifdef HIROH_DEBUG
  for (int i = 0; i < H; i++) {
    for (int j = 0; j < W; j++) {
      switch(board[j][H-1-i]) {
      case WALL:
        std::cerr << '#';
        break;
      case GARBAGE:
        std::cerr << '.';
        break;
      case CLEAR:
        std::cerr << 'X';
      }
    }
    std::cerr << std::endl;
  }
#endif
}

// (x,y)
int dx[] = {-1, 0, 0, 1};
int dy[] = {0, -1, 1, 0};

inline bool can_go(const Point& p) {
  if (p.first < 0 || p.second < 0 || p.first >= W || p.second >= H)
    return false;
  return board[p.first][p.second] != WALL;
}

inline char get_move_symbol(const Point& cur, const Point next) {
  if (std::abs(cur.first - next.first) > 0 &&
      std::abs(cur.second - next.second) > 0) {
    // Diagonal move!
    NOT_REACHED();
  }

  // Move right: (x,y)->(x+1,y)
  if (next.first - cur.first == 1) {
    return 'D';
  }
  // Move left: (x,y)->(x-1,y)
  if (next.first - cur.first == -1) {
    return 'A';
  }
  // Move up: (x,y)->(x,y+1)
  if (next.second - cur.second == 1) {
    return 'W';
  }
  // Move down: (x,y)->(x,y-1)
  if (next.second - cur.second == -1) {
    return 'S';
  }

  // No move.
  if (next.first == cur.first &&
      next.second == cur.second) {
    return ' ';
  }

  NOT_REACHED();
  return '@';
}

std::vector<Point> get_nearest_path(const Point& st,
                                    const Point& dst) {
  Point prev[MAX_L][MAX_L] = {};
  bool passed [MAX_L][MAX_L] = {};
  // Do BFS.
  std::queue<Point> que;
  passed[st.first][st.second] = true;
  prev[st.first][st.second] = Point(-1, -1);
  que.push(st);
  while (!que.empty()) {
    auto t = std::move(que.front());
    que.pop();
    int x = t.first;
    int y = t.second;
    if (x == dst.first && y == dst.second)
      break;

    for (int k = 0; k < 4; k++) {
      int nx = x + dx[k];
      int ny = y + dy[k];
      Point next(nx, ny);
      if (can_go(next) && !passed[nx][ny]) {
        passed[nx][ny] = true;
        prev[nx][ny] = t;
        que.push(std::move(next));
      }
    }
  }

  std::vector<Point> path;
  Point cur = dst;
  while (cur != Point(-1, -1)) {
    path.push_back(cur);
    cur = prev[cur.first][cur.second];
  }

  std::reverse(path.begin(), path.end());
  return path;
}

absl::optional<Point> find_leftmost_garbage_point() {
  for (int x = 0; x < W; x++) {
    for (int y = 0; y < H; y++) {
      if (board[x][y] == GARBAGE) {
        return Point(x,y);
      }
    }
  }
  return absl::nullopt;
}

bool is_at_bottom(const Point& p) {
  Point one_down(p.first, p.second - 1);
  return !can_go(one_down);
}

bool can_go_to_next_line(const Point& p, Point* next_start) {
  const int y_inc = is_at_bottom(p) ? 1 : -1;
  Point cur = p;
  while(can_go(cur)) {
    // next here means => (x+1, y)
    Point next(cur.first + 1, cur.second);
    // If point in next line is not CLEAR yet, go to next line.
    if (can_go(next) && board[next.first][next.second] == GARBAGE) {
      *next_start = next;
      return true;
    }
    cur = Point(cur.first, cur.second + y_inc);
  }
  return false;
}

std::string solution;

void robot_move(const Point& start, const Point dst,
                bool must_neigbor_move = true) {
  print_point(start);
  print_point(dst);
  print_table();
#ifdef HIROH_DEBUG
  std::cerr << solution << std::endl;
#endif
  if (must_neigbor_move) {
    solution += get_move_symbol(start, dst);
    return;
  }

  auto path = get_nearest_path(start, dst);
  for (size_t i = 0; i < path.size() - 1; i++) {
    solution += get_move_symbol(path[i], path[i+1]);
  }

}

Point naive_clean_up(const Point& start) {
  // up->down->up->down
  Point cur = start;
  int y_inc = 1;
  while(true) {
    while (true) {
      // Clear current position.
      board[cur.first][cur.second] = CLEAR;
      Point next = Point(cur.first, cur.second + y_inc);
      if (!can_go(next)) {
        break;
      }
      // Move cur->next
      robot_move(cur, next);
      cur = next;
    }

    Point next_start;
    if (!can_go_to_next_line(cur, &next_start)) {
      return cur;
    }

    // Move cur->next_start.
    robot_move(cur, next_start, false);
    cur = next_start;
    y_inc = -y_inc;
  }
}

int main() {
  // Set All data to WALL.
  std::memset(board, 0, sizeof(board));

  std::cin >> H >> W;
  // board[W][H]
  // board[0][H-1], board[1][H-1], ..., board[W-1][H-1]
  // board[0][H-2], board[1][H-2], ..., board[W-1][H-2]
  // ....
  // board[0][0], board[1][0], ..., board[W-1][0]

  Point robot;

  for (int i = 0; i < H; i++) {
    std::string w;
    std::cin >> w;
    for (int j = 0; j < W; j++) {
      board[j][H-1-i] = w[j] != '#' ? GARBAGE : WALL;
      if (w[j] == 'W')
        robot = Point(j,H-1-i);
    }
  }

  // Algorithm.
  // 1.) Look for move the left most & GARBAGE point.
  // 2.) Clean up->down->up->down from left to right.
  // 3.) Return 1.
  while (true) {
    auto start = find_leftmost_garbage_point();
    if (!start)
      break;
    // move robot ->*start.
    robot_move(robot, *start, false);
    robot = naive_clean_up(*start);
  }

  for (const char c : solution) {
    if (c != ' ')
      std::cout << c;
  }
  std::cout << std::endl;
  std::cerr << "Score: " << solution.length() << std::endl;
  return 0;
}
