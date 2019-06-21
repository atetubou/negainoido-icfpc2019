#include <iostream>
#include <cstdio>
#include "base/ai.h"

int main() {
  AI ai;
  //std::cout << static_cast<int>(ai.get_dir()) << std::endl;
  //ai.turn_CW();
  //std::cout << static_cast<int>(ai.get_dir()) << std::endl;

  printf("%d %d\n", ai.get_pos().first,ai.get_pos().second);

  for (uint32_t i = 0; i < ai.h; i++) {
    for (uint32_t j = 0; j < ai.w; j++) {
      printf("%d", ai.reachable(std::make_pair(i, j)));
    }
    printf("\n");
  }
}
