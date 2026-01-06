#pragma once

#include <filesystem>
#include <fmt/base.h>
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

// TODO: Could we do this without unique_ptr?
// TODO: Lifetimes
class TextureCache
    : public ResourceMap<TextureCache, TextureKey, std::unique_ptr<Image>> {
public:
  const static constexpr size_t MaxTextures = VN_MAXTEXTURES;

  TextureCache();
  ~TextureCache();

  vk::DescriptorSetLayout getDescriptorLayout() { return mTextureLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet.getSet(); }

  Handle insert(std::unique_ptr<Image> image) {
    assert(image != nullptr);
    auto handle =
        ResourceMap<TextureCache, TextureKey, std::unique_ptr<Image>>::insert(
            std::move(image));
    assert(mData[handle.value()] != nullptr);
    updateSet(mData[handle.value()].get(), handle);
    return handle;
  }

  std::unique_ptr<Image> create(const TextureKey& params, Handle index);

private:
  void updateSet(const Image* image, Handle index);

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mTextureLayout;
  DescriptorSet<ImageDescriptor> mDescriptorSet;
  bool mZeroed = false;
};
} // namespace selwonk::vulkan
