#include <iostream>
#include <cstdio>
#include "base/ai.h"

int main() {

  AI ai;
  {
    std::cout << "absolute_manipulator_positions(";
    for (Position&p : ai.get_absolute_manipulator_positions()) {
      std::cout << "(" << p.first << ", " << p.second << "); ";
    }
    std::cout << ")" << std::endl;
  }
  ai.turn_CW();
  {
    std::cout << "absolute_manipulator_positions(";
    for (Position&p : ai.get_absolute_manipulator_positions()) {
      std::cout << "(" << p.first << ", " << p.second << "); ";
    }
    std::cout << ")" << std::endl;
  }
  ai.turn_CW();
  {
    std::cout << "absolute_manipulator_positions(";
    for (Position&p : ai.get_absolute_manipulator_positions()) {
      std::cout << "(" << p.first << ", " << p.second << "); ";
    }
    std::cout << ")" << std::endl;
  }

  printf("%d %d\n", ai.get_pos().first,ai.get_pos().second);

  for (uint32_t i = 0; i < ai.get_height(); i++) {
    for (uint32_t j = 0; j < ai.get_width(); j++) {
      printf("%d", ai.reachable(std::make_pair(i, j)));
    }
    printf("\n");
  }
}
