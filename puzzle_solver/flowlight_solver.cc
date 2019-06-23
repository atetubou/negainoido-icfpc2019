#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include "base/graph.h"
#include "flowlight/main/common.h"
#include <glog/logging.h>
using namespace std;


typedef pair<int, int> P;

int compute_num_vertices(const vector<string> &board) {
  const int L = board.size();
  int num_vertices = 0;
  REP(i, L - 1) REP(j, L - 1) {
    int count = 0;
    REP(k, 2) REP(l, 2) {
      if (board[i + k][j + l] == '#') count++;
    }
    if (count % 2 == 1) {
      num_vertices++;
    }
  }
  return num_vertices;
}

int main() {
  int b_num, e_num, t_size, v_min, v_max, m_num, f_num, d_num, r_num, c_num, x_num;
  int include_num;
  int exclude_num;
  cin >> b_num >> e_num >> t_size >> v_min >> v_max >> m_num >>  f_num >>  d_num >>  r_num >>  c_num >> x_num;
  cin >> include_num >> exclude_num;
  cerr << t_size << endl;

  vector<P> include_squares(include_num);
  vector<P> exclude_squares(exclude_num);
  vector<string> board(t_size + 2, string(t_size + 2, '.'));
  // cerr << board.size() << " " << board[0].size << endl;
  const int L = t_size + 2;
  const int V = L * L;
  REP(i, L) REP(j, L) {
    if (i == 0 || j == 0 || (i + 1 == L) || (j + 1 == L)) {
      board[i][j] = '#';
    }
  }
  

  REP(i, include_num) {
    cin >> include_squares[i].first >> include_squares[i].second;
    include_squares[i].first++;
    include_squares[i].second++;
    board[include_squares[i].second][include_squares[i].first] = 'I';
  }
  REP(i, exclude_num) {
    cin >> exclude_squares[i].first >> exclude_squares[i].second;
    exclude_squares[i].first++;
    exclude_squares[i].second++;
    board[exclude_squares[i].second][exclude_squares[i].first] = '#';
  }

  GraphDistance gd(V);
  const int dh[] = {0, 1, 0, -1};
  const int dw[] = {1, 0, -1, 0};
  REP(h, L) REP(w, L) {
    REP(k, 4) {
      int nh = h + dh[k];
      int nw = w + dw[k];
      if (nh < 0 || L <= nh) continue;
      if (nw < 0 || L <= nw) continue;
      if (board[h][w] == '.' && board[nh][nw] == '.') {
        gd.add_edge(h * L + w, nh * L + w, 1);
      } else if (board[h][w] == '.' && board[nh][nw] == '#') {
        gd.add_edge(h * L + w, nh * L + nw, 1);
      } else if (board[h][w] == '#' && board[nh][nw] == '.') {
        gd.add_edge(h * L + w, nh * L + nw, 1);
      } else if (board[h][w] == '#' && board[nh][nw] == '#') {
        gd.add_edge(h * L + w, nh * L + nw, 0);
      }
    }
  }
  
  vector<int> dist;
  vector<int> parents;
  gd.shortest_path_tree(0, dist, parents);


  for (const auto &p: exclude_squares) {
    int h = p.second;
    int w = p.first;
    LOG_IF(FATAL, dist[h * L + w] < 0);
    while(h != 0 || w != 0) {
      int next = parents[h * L + w];
      h = next / L;
      w = next % L;
      board[h][w] = '#';
    }
  }

  REP(i, L) REP(j, L) {
    if (board[i][j] == 'I') board[i][j] = '.';
  }

  vector<char> items;
  items.push_back('W');
  REP(i, m_num) items.push_back('B');
  REP(i, f_num) items.push_back('F');
  REP(i, d_num) items.push_back('L');
  REP(i, r_num) items.push_back('R');
  REP(i, c_num) items.push_back('C');
  REP(i, x_num) items.push_back('X');
  size_t cur = 0;
  REP(i, L) REP(j, L) {
    if (board[i][j] == '.' && cur < items.size()) {
      board[i][j] = items[cur++];
    }
  }
  
  int num_vertices = compute_num_vertices(board);
  LOG_IF(FATAL, num_vertices > v_max);

  REP2(h, 1, L - 1) {
    REP2(w, 1, L - 1) {
      if (num_vertices >= v_min) {
        break;
      }
      
      int total_count = 0;
      map<int, int> dh_count;
      map<int, int> dw_count;
      for (int dh = -1; dh <= 1; dh++) {
        for (int dw = -1; dw <= 1; dw++) {
          int nh = dh + h;
          int nw = dw + w;
          if (board[nh][nw] == '.') {
            total_count++;
            dh_count[dh]++;
            dw_count[dw]++;            
          }
        }
      }
      if (total_count == 6 && (dh_count[1] == 0 || dh_count[-1] == 0 || 
                               dw_count[1] == 0 || dw_count[-1] == 0)) {
        board[h][w] = '#';
        num_vertices += 4;
      }
    }
  }

  cout << L - 2 << " "<< L - 2 << endl;
  REP2(i, 1, t_size + 1) {
    cout << string(board[L - i - 1].begin() + 1, board[L - i - 1].end() - 1) << endl;
  }
}
