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

  ASSERT_TRUE(var.setString("-1.45"));
  ASSERT_NEAR(var.value(), -1.45f, 0.01f);
  TEST_PARSE_OK(var, "-145", -145);

  TEST_PARSE_FAIL(var, "a.456", -145);
  TEST_PARSE_FAIL(var, "789a", -145);
  TEST_PARSE_FAIL(var, "1.2.3", -145);
  TEST_PARSE_FAIL(var, " 12 ", -145);
}

// Values are deliberatly nonsensical to catch issues from gaps
enum class TestEnum {
  First = 1,
  Gap = 10,
  Null = 100,
};

Cvar::Enum<TestEnum> mkTestEnum() {
  return Cvar::Enum<TestEnum>("testing.enum", TestEnum::Null, "Testing enum",
    {
        {"Null", "n", TestEnum::Null},
        {"First", "f", TestEnum::First},
        {"Gap", "f", TestEnum::Gap},
    });
}

TEST(Cvar, CreatesEnum) {
  auto var = mkTestEnum();
}

TEST(Cvar, ParsesEnum) {
  auto var = mkTestEnum();

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
  auto var = mkTestEnum();

  ASSERT_EQ(var.getDefaultText(), "Gap");
}

#define CHECK_CLEAN \
  ASSERT_FALSE(ivar.dirty()); \
  ASSERT_FALSE(evar.dirty());
#define CHECK_DIRTY \
  ASSERT_TRUE(ivar.dirty()); \
  ASSERT_TRUE(evar.dirty());
TEST(Cvar, SetPendingSetsDirty) {
  auto ivar = Cvar::Int("testing.int", 0, "Test int");
  auto evar = mkTestEnum();

  CHECK_CLEAN;

  // Setting pending values should make the var dirty until it is applied
  ivar.setPendingValue(100);
  evar.setPendingInt((int)TestEnum::Gap);
  CHECK_DIRTY;

  ivar.apply();
  evar.apply();
  CHECK_CLEAN;

  // Exception: Setting a pending value to the current value does not dirty the var
  ivar.setPendingValue(ivar.value());
  evar.setPendingInt((int)evar.value());
  CHECK_CLEAN;
}
#undef CHECK_CLEAN
#undef CHECK_DIRTY

} // namespace selwonk::core::test
