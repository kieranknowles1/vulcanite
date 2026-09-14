set(SOURCES
  core/cvar.cpp
  core/vfs.cpp
)

vn_add_executable(test-vncore ${SOURCES})
target_link_libraries(test-vncore vncore GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test-vncore)
