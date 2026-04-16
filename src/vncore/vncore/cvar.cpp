#include "cvar.hpp"

#include <fmt/base.h>

namespace selwonk::core {

bool Cvar::parseCli(int argc, char** argv) {
  if (argc <= 1)
    return false;                  // No args
  std::string_view arg1 = argv[1]; // argv[0] is the process name
  if (arg1 == "-h" || arg1 == "--help" || arg1 == "help") {
    fmt::println("Usage: {} [name value]... -- set CVars on startup", argv[0]);
    fmt::println("known CVars:");
    for (auto& var : mVars) {
      // TODO: Display/parse string values for enum options
      fmt::println("  {} = {}: {}", var.second->getName(),
                   var.second->toString(), var.second->getDescription());
    }
    return true;
  }

  if (argc % 2 != 1) {
    fmt::println("Expected arguments to follow [name value]");
    return true;
  }

  bool bad = false;
  int count = (argc - 1) / 2;
  for (int i = 0; i < count; i++) {
    auto name = argv[i * 2 + 1];
    auto value = argv[i * 2 + 2];

    auto var = mVars.find(name);
    if (var == mVars.end()) {
      fmt::println("Unknown CVar '{}'", name);
      bad = true;
    }

    bool ok = var->second->setString(value);
    if (!ok) {
      fmt::println("Invalid value '{}' for '{}'", value, name);
      bad = true;
    }
  }
  return bad;
}

} // namespace selwonk::core
