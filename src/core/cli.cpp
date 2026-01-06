#include "cli.hpp"

#include <fmt/base.h>
#include <stdexcept>

namespace selwonk::core {
Cli::Cli(int argc, char** argv) {
  parser.addOption({"-h", "--help", "Display help then exit", &help});
  parser.addOption({
      "-q",
      "--quit-after",
      "Run for the specified number of frames, then quit",
      &quitAfterFrames,
  });

  parser.parse(argc, argv);
}

int Cli::Option::parse(int argc, char** argv) {
  auto visitor = [&](const auto& option) {
    using T = std::decay_t<decltype(option)>;

    if constexpr (std::is_same_v<T, bool*>) {
      *option = true;
      return 1;
    } else if constexpr (std::is_same_v<T, std::optional<unsigned int>*>) {
      if (argc < 2)
        throw std::runtime_error("Expected a value");
      *option = std::stoi(argv[1]);
      return 2;
    } else {
      static_assert(false, "non-exhaustive visitor");
    }
  };

  return std::visit(visitor, target);
}

void Cli::Parser::parse(int argc, char** argv) {
  int i = 1; // Skip program name
  while (i < argc) {
    std::string_view name(argv[i]);
    auto opt = mOptions.find(name);
    if (opt == mOptions.end()) {
      throw std::runtime_error("Unknown argument " + std::string(name));
    }
    i += opt->second.parse(argc - i, argv + i);
  }
}

void Cli::Parser::printHelp() {
  fmt::println("Usage:");
  for (auto& opt : mOrderedOptions) {
    fmt::println("  {}, {}: {}", opt.shortName, opt.longName, opt.description);
  }
}

} // namespace selwonk::core
