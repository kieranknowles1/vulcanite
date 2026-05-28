#include "cvar.hpp"

#include <fmt/base.h>
#include <spdlog/spdlog.h>

namespace selwonk::core {

bool Cvar::parseCli(int argc, char** argv) {
  if (argc <= 1)
    return false;                  // No args
  std::string_view arg1 = argv[1]; // argv[0] is the process name
  if (arg1 == "-h" || arg1 == "--help" || arg1 == "help") {
    fmt::println("Usage: {} [name value]... -- set CVars on startup", argv[0]);
    fmt::println("known CVars:");
    for (auto& var : mVars) {
      fmt::println("  {} = {}: {}", var.second->getName(),
                   var.second->toString(), var.second->getDescription());
    }
    return true;
  }

  if (argc % 2 != 1) {
    SPDLOG_ERROR("Expected arguments to follow [name value]");
    return true;
  }

  bool bad = false;
  int count = (argc - 1) / 2;
  for (int i = 0; i < count; i++) {
    auto name = argv[i * 2 + 1];
    auto value = argv[i * 2 + 2];

    auto var = mVars.find(name);
    if (var == mVars.end()) {
      SPDLOG_ERROR("Unknown CVar '{}'", name);
      bad = true;
    }

    bool ok = var->second->setString(value);
    if (!ok) {
      SPDLOG_ERROR("Invalid value '{}' for '{}'", value, name);
      bad = true;
    }
  }
  return bad;
}

} // namespace selwonk::core
