#pragma once

#include <filesystem>
#include <vulkan/vulkan.hpp>

#include "image.hpp"
#include "resourcemap.hpp"
#include "shader.hpp"

namespace selwonk::vulkan {

struct GltfImage {
  std::filesystem::path source;
  std::string name;
};
using TextureKey = std::variant<GltfImage, std::filesystem::path>;

class TextureCache : public ResourceMap<TextureCache, TextureKey, Image> {
public:
  const static constexpr size_t MaxTextures = VN_MAXTEXTURES;

  TextureCache();
  ~TextureCache();

  vk::DescriptorSetLayout getDescriptorLayout() { return mTextureLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet.getSet(); }

  vk::Sampler create(const TextureKey& params, Handle index);

private:
  void updateSet(const Image& image, Handle index);

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mTextureLayout;
  DescriptorSet<ImageDescriptor> mDescriptorSet;
  bool mZeroed = false;
};
} // namespace selwonk::vulkan
