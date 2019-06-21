#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>
#include <gflags/gflags.h>

DEFINE_string(problem, "","");

using board = std::vector<std::string>;

int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
char dir[] = "WSDA";

void dfs(int sx, int sy, board* b, int* cnt) {
  (*b)[sy][sx] = '#';
  --*cnt;
  if (*cnt == 0) return;

  int w = (*b)[0].size();
  int h = b->size();

  for (int i = 0; i < 4; ++i){
    int nx = sx + dx[i];
    int ny = sy + dy[i];

    if (nx < 0 || ny < 0 || w <= nx || h <= ny) continue;

    if ((*b)[ny][nx] == '#') continue;
    std::cout << dir[i];
    dfs(nx, ny, b, cnt);
    std::cout << dir[i^1];
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();


  std::ifstream fin(FLAGS_problem);

  int h, w;
  fin >> h >> w;
  std::vector<std::string> in(h);
  for (int i = 0; i < h; ++i) {
    fin >> in[i];
  }

  std::reverse(in.begin(), in.end());

  int sx = 0, sy = 0;

  int cnt = 0;

  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      if (in[i][j] != '#') {
	++cnt;
      }
      if (in[i][j] != 'W') {
	continue;
      }
      sx = j;
      sy = i;
    }
  }

  dfs(sx, sy, &in, &cnt);
  std::cout << std::endl;
}
