#pragma once

#include <map>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace selwonk::core {
class Cli {
public:
  class Option {
  public:
    std::string_view shortName;
    std::string_view longName;
    std::string_view description;

    std::variant<std::optional<unsigned int>*, bool*> target;

    // Parse an argument, given the argument and all following parameters
    // Return the number of consumed parameters
    int parse(int argc, char** argv);
  };

  class Parser {
  public:
    void addOption(const Option& option) {
      mOrderedOptions.emplace_back(option);
      mOptions[option.shortName] = option;
      mOptions[option.longName] = option;
    }

    void parse(int argc, char** argv);
    void printHelp();

  private:
    // Options in declaration order
    std::vector<Option> mOrderedOptions;
    // Options structured for fast lookup
    std::map<std::string_view, Option> mOptions;
  };

  Cli(int argc, char** argv);

  bool help;
  std::optional<unsigned int> quitAfterFrames;

  Parser parser;
};
} // namespace selwonk::core
