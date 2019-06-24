#ifndef KONMARI_AI_SOLVER_H
#define KONMARI_AI_SOLVER_H

#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <set>
#include <map>

#include "base/ai.h"

class KonmariAISolver {
 public:
  KonmariAISolver(const std::pair<int,int>& initial,
                  const std::vector<std::string>& board,
                  const std::vector<std::vector<bool>>& filled,
                  const std::set<std::pair<int,int>>& area
                  /*, std::map<char, int> extensions*/);
  ~KonmariAISolver();

  std::vector<std::vector<Command>> solve();
 private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

#endif
