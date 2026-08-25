#include <gtest/gtest.h>

#include <vncore/cvar.hpp>

namespace selwonk::core::test {

// Assert that parsing a valid string works and updates the value
#define TEST_PARSE_OK(var, str, expected)                                      \
  ASSERT_TRUE(var.setString(str));                                             \
  ASSERT_EQ(var.value(), expected);                                            \
  ASSERT_EQ(*var.getPendingValue(), expected);

// Assert that parsing an invalid string fails and does not update the value
#define TEST_PARSE_FAIL(var, str, old)                                         \
  ASSERT_FALSE(var.setString(str));                                            \
  ASSERT_EQ(var.value(), old);                                                 \
  ASSERT_EQ(*var.getPendingValue(), old);

TEST(Cvar, ParsesInt) {
  Cvar::Int var("testing.int", 0, "Testing int");

  TEST_PARSE_OK(var, "123", 123);

  // Value must be an integer
  TEST_PARSE_FAIL(var, "400.5", 123);

  // Value must contain only the number
  auto cases = {"a456", "456a", " 400 ", "230 2"};
  for (auto& test : cases) {
    TEST_PARSE_FAIL(var, test, 123);
  }

  // An unsigned int must be >= 0
  Cvar::Int uvar("testing.uint", 0, "Testing uint", Cvar::Flags::Unsigned);
  TEST_PARSE_OK(uvar, "0", 0);
  TEST_PARSE_FAIL(uvar, "-1", 0);
}

TEST(Cvar, ParsesFloat) {
  Cvar::Float var("testing.float", 0, "Testing float");

  ASSERT_TRUE(var.setString("1.23"));
  ASSERT_NEAR(var.value(), 1.23f, 0.01f);
  ASSERT_TRUE(var.setString("456"));
  ASSERT_EQ(var.value(), 456);

  auto cases = {"a4.56", "789a", "1.2.3", " 12 "};
  for (auto& test : cases) {
    TEST_PARSE_FAIL(var, test, 456);
  }
}

enum class TestEnum {
  First = 1,
  Gap = 10,
};

TEST(Cvar, CreatesEnum) {
  Cvar::Enum<TestEnum> var("testing.enum", TestEnum::First, "Testing enum",
                           {
                               {"First", "f", TestEnum::First},
                               {"Gap", "f", TestEnum::Gap},
                           });
}

TEST(Cvar, ParsesEnum) {
  Cvar::Enum<TestEnum> var("testing.enum", TestEnum::First, "Testing enum",
                           {
                               {"First", "f", TestEnum::First},
                               {"Gap", "f", TestEnum::Gap},
                           });

  ASSERT_TRUE(var.setString("First"));
  ASSERT_EQ(var.value(), TestEnum::First);

  // Gaps in the enum value should not affect parsing
  ASSERT_TRUE(var.setString("Gap"));
  ASSERT_EQ(var.value(), TestEnum::Gap);
  ASSERT_EQ(var.getPendingInt(), (int)var.value());

  // Invalid string should not be accepted
  ASSERT_FALSE(var.setString("Invalid"));
  ASSERT_EQ(var.value(), TestEnum::Gap);
}

TEST(Cvar, EnumToString) {
  Cvar::Enum<TestEnum> var("testing.enum", TestEnum::Gap, "Testing enum",
                           {
                               {"First", "f", TestEnum::First},
                               {"Gap", "f", TestEnum::Gap},
                           });

  ASSERT_EQ(var.getDefaultText(), "Gap");
}

} // namespace selwonk::core::test
