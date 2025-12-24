#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

namespace selwonk {
// Very basic VFS implementation. Absolutely will not scale to many providers
// and probably will need refactoring for different provider types (i.e., zip)
class Vfs {
public:
  // A path within the VFS
  using Path = const std::filesystem::path &;
  // A path within an asset-type subdirectory
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
  const static std::filesystem::path Meshes;

  Vfs(Providers providers) : mProviders(std::move(providers)) {}

  static std::filesystem::path getExePath();

  std::ifstream open(Path path);
  void readfull(Path path, std::vector<std::byte> &buffer);

private:
  Providers mProviders;
};
} // namespace selwonk
