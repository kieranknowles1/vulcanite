#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "../../assets/shaders/interop.h"
#include "../vfs.hpp"
#include "buffer.hpp"
#include "fastgltf/core.hpp"

namespace selwonk::vulkan {
class Mesh {
public:
  struct Data {
    std::vector<uint32_t> indices;
    std::vector<interop::Vertex> vertices;
  };

  class LoadException : public std::exception {
  public:
    LoadException(fastgltf::Error error) : mError(error) {}
    fastgltf::Error mError;

    const char* what() const noexcept override {
      return fastgltf::getErrorMessage(mError).data();
    }
  };

  struct Surface {
    uint32_t mStartIndex;
    uint32_t mCount;
  };

  // A GLTF can contain multiple meshes, each with multiple submeshes
  static std::vector<std::shared_ptr<Mesh>> load(Vfs::SubdirPath path);

  Mesh(std::string_view name, Data data);
  ~Mesh();

  // TODO: Make these private
  // private:
  std::string name;
  size_t mIndexCount;

  Buffer mIndexBuffer;
  Buffer mVertexBuffer;

  static constexpr std::string_view AttrPosition = "POSITION";
  static constexpr std::string_view AttrNormal = "NORMAL";
  static constexpr std::string_view AttrUv = "TEXCOORD_0";
  static constexpr std::string_view AttrColor = "COLOR_0";
};
} // namespace selwonk::vulkan
