#include "json/json.h"

#include <string>

#include <gtest/gtest.h>

#include "absl/strings/string_view.h"

TEST(JsonTest, Simple) {
  Json::CharReaderBuilder builder;
  auto* reader = builder.newCharReader();

  Json::Value value;
  std::string err;
  static constexpr absl::string_view json = R"(
{"a": 1}
)";

  ASSERT_TRUE(reader->parse(json.begin(), json.end(), &value, &err));
  EXPECT_TRUE(err.empty());

  EXPECT_TRUE(value.isMember("a"));
  EXPECT_TRUE(value["a"].isInt());
  EXPECT_EQ(value["a"].asInt(), 1);
}
