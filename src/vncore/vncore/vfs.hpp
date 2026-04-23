#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>

namespace selwonk::core {
// Very basic VFS implementation. Absolutely will not scale to many providers
// and probably will need refactoring for different provider types (i.e., zip)
// Probably want to list all files on init and build a lookup table
class Vfs {
public:
  class File {
  public:
    virtual ~File() = default;
    // Open the file as a stream
    virtual std::ifstream open() = 0;

    // Get the file's name
    virtual const char* name() = 0;

    // Read a file in its entirity
    void readfull(std::vector<std::byte>& buffer);
  };
  using FilePtr = std::unique_ptr<File>;

  // A path within the VFS
  using Path = const std::filesystem::path&;

  class Provider {
  public:
    virtual FilePtr open(Path path) = 0;
    virtual ~Provider() = default;
  };

  class FilesystemProvider : public Provider {
  public:
    class FilesystemFile : public File {
    public:
      FilesystemFile(std::filesystem::path path) : mPath(path) {}
      std::ifstream open() { return std::ifstream(mPath); }
      const char* name() { return mPath.c_str(); }

    private:
      std::filesystem::path mPath;
    };

    FilesystemProvider(const std::filesystem::path& root) : root(root) {}

    FilePtr open(Path path) override {
      return std::make_unique<FilesystemFile>(root / path);
    }
    ~FilesystemProvider() override = default;

  private:
    std::filesystem::path root;
  };

  using Providers = std::vector<std::unique_ptr<Provider>>;

  Vfs(Providers providers) : mProviders(std::move(providers)) {}

  FilePtr get(Path path);
  void readfull(Path path, std::vector<std::byte>& buffer);

private:
  Providers mProviders;
};
} // namespace selwonk::core
