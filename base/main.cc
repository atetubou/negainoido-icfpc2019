#include <iostream>
#include "base/ai.h"

int main() {
  AI ai;
  std::cout << static_cast<int>(ai.get_dir()) << std::endl;
  ai.turn_CW();
  std::cout << static_cast<int>(ai.get_dir()) << std::endl;
}
