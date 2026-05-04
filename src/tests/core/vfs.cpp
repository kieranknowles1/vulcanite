#include <gtest/gtest.h>

#include <vncore/vfs.hpp>

#include "util.hpp"

namespace selwonk::core::test {

// Demonstrate some basic assertions.
TEST(vfs, ReadNewline) {
  std::string_view dummyData = "\r \n \r\n \n\r";
  std::vector<char> data;
  for (auto byte : dummyData) {
    data.push_back(byte);
  }

  auto tmpDir = selwonk::test::util::tempDir();
  auto file = selwonk::test::util::writeDummyFile(data);

  Vfs::Providers providers;
  providers.push_back(std::make_unique<Vfs::FilesystemProvider>(tmpDir));
  auto vfs = core::Vfs(std::move(providers));

  std::vector<char> out;
  vfs.get(file)->readfull(out);
  std::filesystem::remove(file);

  ASSERT_EQ(data, out);
}

}