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
    virtual const char* c_str() = 0;

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
      FilesystemFile(std::filesystem::path path) : mPath(path) {
#ifdef _WIN32
          // TODO: This is an ugly workaround for windows using utf16 paths
          mPathStr = mPath.string();
#endif
      }
      std::ifstream open() {
        // TODO: Test case, this shouldn't mess up line endings (windows moment)
        std::ifstream ptr(mPath, std::ios::binary);
        if (!ptr.is_open()) {
          // TODO: Don't return an invalid file from FilesystemProvider and change this to assert
          throw std::runtime_error("Failed to open file");
        }
        return ptr;
      }
      const char* c_str() {
#ifdef _WIN32
          return mPathStr.c_str();
#else
          return mPath.c_str();
#endif
      }

    private:
      std::filesystem::path mPath;
#ifdef _WIN32
      std::string mPathStr;
#endif
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
