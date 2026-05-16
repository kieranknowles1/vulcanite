#pragma once

#include <vulkan/vulkan.hpp>

#include "vncore/handlelist.hpp"
#include <vncore/cvar.hpp>
#include <vnvulkan/image.hpp>
#include <vnvulkan/shader.hpp>

namespace selwonk::vulkan {

// TODO: Lifetimes
class TextureManager {
public:
  using Handle = core::HandleList<Image>::Handle;

  size_t size() { return mData.size(); }

  TextureManager(core::Cvar::Int& maxTextures);
  ~TextureManager();

  vk::DescriptorSetLayout getDescriptorLayout() { return mTextureLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet; }

  Handle insert(Image& image) {
    auto handle = mData.insert(std::move(image));
    updateSet(&mData.get(handle), handle);
    return handle;
  }

  void incRef(Handle handle) { mData.incRef(handle); }
  void decRef(Handle handle) { mData.decRef(handle); }

  Handle getWhite() const { return mWhite; }
  Handle getMissing() const { return mMissing; }

  int getCapacity() const { return mCapacity; }

  const Image& getTexture(Handle handle) { return mData.get(handle); }

private:
  void updateSet(const Image* image, Handle index);
  void resize(int capacity);

  core::HandleList<Image> mData;

  int mCapacity;

  Handle mWhite;
  Handle mMissing;

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mTextureLayout;
  vk::DescriptorSet mDescriptorSet;
};
} // namespace selwonk::vulkan
