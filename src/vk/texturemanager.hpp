#pragma once

#include <filesystem>
#include <fmt/base.h>
#include <vulkan/vulkan.hpp>

#include "../core/cvar.hpp"
#include "image.hpp"
#include "resourcemap.hpp"
#include "shader.hpp"

namespace selwonk::vulkan {

// TODO: Lifetimes
class TextureManager
    : public ResourceMap<TextureManager, std::filesystem::path, Image> {
public:
  TextureManager(core::Cvar::Int& maxTextures);
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

  Handle getWhite() const { return mWhite; }
  Handle getMissing() const { return mMissing; }

  int getCapacity() const { return mCapacity; }

private:
  void updateSet(const Image* image, Handle index);
  void resize(int capacity);

  int mCapacity;

  Handle mWhite;
  Handle mMissing;

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mTextureLayout;
  DescriptorSet<ImageDescriptor> mDescriptorSet;
};
} // namespace selwonk::vulkan
