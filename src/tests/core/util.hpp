#pragma once

#include <vector>
#include <filesystem>

namespace selwonk::test::util {

std::string randomString(std::size_t length);

std::filesystem::path tempDir();

std::filesystem::path writeDummyFile(const std::vector<char>& data);

}