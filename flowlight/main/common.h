#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <cstdlib>
#include <iostream>

#define REP2(i, m, n) for(int i = (int)(m); i < (int)(n); i++)
#define REPD(i, n) for(int i = (int)(n) - 1; i >= 0; i--)
#define REP(i, n) REP2(i, 0, n)
#define ALL(c) (c).begin(), (c).end()
#define PB(e) push_back(e)
#define FOREACH(i, c) for(auto i = (c).begin(); i != (c).end(); ++i)
#define MP(a, b) make_pair(a, b)
#define BIT(n, m) (((n) >> (m)) & 1)

typedef long long ll;

template <typename S, typename T> std::ostream &operator<<(std::ostream &out, const std::pair<S, T> &p) {
  out << "(" << p.first << ", " << p.second << ")";
  return out;
}

template <typename T> std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
  out << "[";
  REP(i, v.size()){
    if (i > 0) out << ", ";
    out << v[i];
  }
  out << "]";
  return out;
}



#endif /* COMMON_H */
