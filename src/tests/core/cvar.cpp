#include <cstdlib>
#include <gtest/gtest.h>

#include <vncore/cvar.hpp>

namespace selwonk::core::test {

// Values are deliberatly non contiguous to catch issues from gaps
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

TEST(Cvar, ReadsFromEnvironment) {
  // TODO: Use template functions for cvar test cases
  setenv("VN_TESTING_INT", "123", true);
  setenv("VN_TESTING_BOOL", "1", true);
  setenv("VN_TESTING_FLOAT", "100.5", true);
  setenv("VN_TESTING_ENUM", "Gap", true);

  Cvar::Int ivar("testing.int", 0, "test int");
  Cvar::Bool bvar("testing.bool", false, "test bool");
  Cvar::Float fvar("testing.float", 0.0f, "test float");
  Cvar::Enum evar = mkTestEnum();

  // Value init should not read env
  ASSERT_EQ(ivar.value(), 0);
  ASSERT_EQ(bvar.value(), false);
  ASSERT_EQ(fvar.value(), 0.0f);
  ASSERT_EQ(evar.value(), TestEnum::Null);

  ASSERT_TRUE(ivar.setFromEnvironment());
  ASSERT_TRUE(bvar.setFromEnvironment());
  ASSERT_TRUE(fvar.setFromEnvironment());
  ASSERT_TRUE(evar.setFromEnvironment());

  // Set from env should respect string parsing rules and not mutate if invalid
  setenv("VN_TESTING_INT", " abcd", true);
  ASSERT_FALSE(ivar.setFromEnvironment());

  ASSERT_EQ(ivar.value(), 123);
  ASSERT_EQ(bvar.value(), true);
  ASSERT_EQ(fvar.value(), 100.5);
  ASSERT_EQ(evar.value(), TestEnum::Gap);

  // Should be parsed along with CLI. CLI should take priority
  ivar.setString("0");
  setenv("VN_TESTING_INT", "100", true);
  std::array<const char*, 1> emptyCli = {"vulcanite"};
  ASSERT_FALSE(Cvar::get().parseCli(emptyCli.size(), emptyCli.data()));
  ASSERT_EQ(ivar.value(), 100);
  std::array<const char*, 3> fullCli = {"vulcanite", "testing.int", "400"};
  ASSERT_FALSE(Cvar::get().parseCli(fullCli.size(), fullCli.data()));
  ASSERT_EQ(ivar.value(), 400);

  // set from env is a no-op if env is unset
  unsetenv("VN_TESTING_INT");
  ASSERT_TRUE(ivar.setFromEnvironment());
  ASSERT_EQ(ivar.value(), 400);
}

TEST(Cvar, CreatesEnum) { auto var = mkTestEnum(); }

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

  ASSERT_EQ(var.getDefaultText(), "Null");
}

#define CHECK_CLEAN                                                            \
  ASSERT_FALSE(ivar.dirty());                                                  \
  ASSERT_FALSE(evar.dirty());
#define CHECK_DIRTY                                                            \
  ASSERT_TRUE(ivar.dirty());                                                   \
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

  // Exception: Setting a pending value to the current value does not dirty the
  // var
  ivar.setPendingValue(ivar.value());
  evar.setPendingInt((int)evar.value());
  CHECK_CLEAN;
}
#undef CHECK_CLEAN
#undef CHECK_DIRTY

} // namespace selwonk::core::test
