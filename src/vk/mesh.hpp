#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "../../assets/shaders/interop.h"
#include "../vfs.hpp"
#include "buffer.hpp"
#include "fastgltf/core.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {
class Mesh {
public:
  class LoadException : public std::exception {
  public:
    LoadException(fastgltf::Error error) : mError(error) {}
    fastgltf::Error mError;

    const char *what() const noexcept override {
      return fastgltf::getErrorMessage(mError).data();
    }
  };

  struct Surface {
    uint32_t mStartIndex;
    uint32_t mCount;
  };

  // A GLTF can contain multiple meshes, each with multiple submeshes
  static std::vector<Mesh> load(VulkanHandle &handle, Vfs::SubdirPath path);

  void upload(VulkanHandle &handle);
  void free(VulkanHandle &handle);

  // TODO: Private, mesh loading
  // private:
  std::string mName;

  Buffer mIndexBuffer;
  Buffer mVertexBuffer;

  std::vector<uint32_t> mIndices;
  std::vector<interop::Vertex> mVertices;
  std::vector<Surface> mSubMeshes;

  static constexpr std::string_view AttrPosition = "POSITION";
  static constexpr std::string_view AttrNormal = "NORMAL";
  static constexpr std::string_view AttrUv = "TEXCOORD_0";
  static constexpr std::string_view AttrColor = "COLOR_0";
};
} // namespace selwonk::vulkan
