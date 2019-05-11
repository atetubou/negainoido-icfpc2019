#include <gtest/gtest.h>

#include "sandbox/fib.h"

TEST(FibTest, Simple) {
  EXPECT_EQ(fib(0), 1);
  EXPECT_EQ(fib(1), 1);
  EXPECT_EQ(fib(2), 2);
  EXPECT_EQ(fib(3), 3);
  EXPECT_EQ(fib(4), 5);
  EXPECT_EQ(fib(5), 8);
}
