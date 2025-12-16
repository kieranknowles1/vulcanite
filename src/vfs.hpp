#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

namespace selwonk {
// Very basic VFS implementation. Absolutely will not scale to many providers
// and probably will need refactoring for different provider types (i.e., zip)
class Vfs {
public:
  using Path = const std::filesystem::path &;
  using SubdirPath = const std::filesystem::path &;

  class Provider {
  public:
    virtual std::ifstream open(const std::filesystem::path &path) = 0;
    virtual ~Provider() = default;
  };

  class FilesystemProvider : public Provider {
  public:
    FilesystemProvider(const std::filesystem::path &root) : root(root) {}

    std::ifstream open(Path path) override {
      return std::ifstream(root / path);
    }
    ~FilesystemProvider() override = default;

  private:
    std::filesystem::path root;
  };

  using Providers = std::vector<std::unique_ptr<Provider>>;

  const static std::filesystem::path Shaders;

  Vfs(Providers providers) : mProviders(std::move(providers)) {}

  static std::filesystem::path getExePath();

  std::ifstream open(Path path);

private:
  Providers mProviders;
};
} // namespace selwonk
