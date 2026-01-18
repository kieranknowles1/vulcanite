#pragma once

#include <filesystem>
#include <fmt/base.h>
#include <vulkan/vulkan.hpp>

#include "image.hpp"
#include "resourcemap.hpp"
#include "shader.hpp"

namespace selwonk::vulkan {

// TODO: Lifetimes
class TextureManager
    : public ResourceMap<TextureManager, std::filesystem::path, Image> {
public:
  const static constexpr size_t MaxTextures = VN_MAXTEXTURES;

  TextureManager();
  ~TextureManager();

  vk::DescriptorSetLayout getDescriptorLayout() { return mTextureLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet.getSet(); }

  Handle insert(Image image) {
    auto handle =
        ResourceMap<TextureManager, std::filesystem::path, Image>::insert(
            std::move(image));
    updateSet(&mData[handle.value()], handle);
    return handle;
  }

  Image create(const std::filesystem::path& params, Handle index);

  Handle getWhite() { return mWhite; }
  Handle getMissing() { return mMissing; }

private:
  void updateSet(const Image* image, Handle index);

  Handle mWhite;
  Handle mMissing;

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mTextureLayout;
  DescriptorSet<ImageDescriptor> mDescriptorSet;
};
} // namespace selwonk::vulkan
