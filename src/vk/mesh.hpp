#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "../../assets/shaders/interop.h"
#include "buffer.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {
class Mesh {
public:
  void upload(VulkanHandle &handle);
  void free(VulkanHandle &handle);

  // TODO: Private, mesh loading
  // private:
  Buffer mIndexBuffer;
  Buffer mVertexBuffer;

  std::vector<uint32_t> mIndices;
  std::vector<interop::Vertex> mVertices;
};
} // namespace selwonk::vulkan
