#include "vulkannativehandleprovider.hpp"

#include <vncore/cvar.hpp>

namespace selwonk::vulkan {

core::Cvar::Int VulkanNativeHandleProvider::MaxTextures("render.max_textures", 8192,
  "Maximum number of textures",
  core::Cvar::Flags::Unsigned);


VulkanNativeHandleProvider::VulkanNativeHandleProvider() 
  : mTextures(MaxTextures) {

}

}