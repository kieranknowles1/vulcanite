#include <gtest/gtest.h>

#include <vncore/cvar.hpp>

namespace selwonk::core::test {

TEST(Cvar, ParsesInt) {
  Cvar::Int var("testing.int", 0, "Testing int");

  ASSERT_TRUE(var.setString("123"));
  ASSERT_EQ(var.value(), 123);

  ASSERT_FALSE(var.setString("a456"));
  ASSERT_EQ(var.value(), 123);

  ASSERT_FALSE(var.setString("789a"));
  ASSERT_EQ(var.value(), 123);
}

TEST(Cvar, ParsesFloat) {
  Cvar::Float var("testing.float", 0, "Testing float");

  ASSERT_TRUE(var.setString("1.23"));
  ASSERT_NEAR(var.value(), 1.23f, 0.01f);

  ASSERT_FALSE(var.setString("a4.56"));
  ASSERT_NEAR(var.value(), 1.23f, 0.01f);

  ASSERT_FALSE(var.setString("7.89a"));
  ASSERT_NEAR(var.value(), 1.23f, 0.01f);
}

enum class TestEnum {
  First = 1,
  Gap = 10,
};

TEST(Cvar, ParsesEnum) {
  Cvar::Enum<TestEnum> var("testing.enum", TestEnum::First, "Testing enum",
                           {
                               {"First", "f", TestEnum::First},
                               {"Gap", "f", TestEnum::Gap},
                           });

  ASSERT_TRUE(var.setString("First"));
  ASSERT_EQ(var.value(), (int)TestEnum::First);

  ASSERT_TRUE(var.setString("Gap"));
  ASSERT_EQ(var.value(), (int)TestEnum::Gap);

  ASSERT_FALSE(var.setString("Invalid"));
  ASSERT_EQ(var.value(), (int)TestEnum::Gap);
}

} // namespace selwonk::core::test
