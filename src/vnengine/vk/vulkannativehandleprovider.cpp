#include "vulkannativehandleprovider.hpp"

#include <vncore/cvar.hpp>

namespace selwonk::vulkan {

core::Cvar::Int VulkanNativeHandleProvider::MaxTextures("render.max_textures", 8192,
  "Maximum number of textures",
  core::Cvar::Flags::Unsigned);

core::Cvar::Int VulkanNativeHandleProvider::MaxVertexBuffers("render.max_vertex_buffers", 8192,
  "Maximum number of vertex buffers",
  core::Cvar::Flags::Unsigned);


VulkanNativeHandleProvider::VulkanNativeHandleProvider() 
  : mTextures(MaxTextures), mIndexBuffers(MaxVertexBuffers), mVertexBuffers(MaxVertexBuffers) {

}

}