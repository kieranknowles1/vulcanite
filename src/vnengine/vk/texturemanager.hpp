#pragma once

#include <vulkan/vulkan.hpp>

#include "image.hpp"
#include "shader.hpp"
#include "vncore/handlelist.hpp"
#include <vnassets/image.hpp>
#include <vncore/cvar.hpp>

namespace selwonk::vulkan {

// TODO: Lifetimes
class TextureManager {
public:
  using Handle = assets::ImageBase::Handle;

  struct LoadJob : core::ThreadPool::Job {
    LoadJob(Handle* out, const fastgltf::Asset& asset,
            const fastgltf::DataSource& data)
        : out(out), asset(asset), data(data) {}
    void execute() override;

    Handle* out;
    const fastgltf::Asset& asset;
    const fastgltf::DataSource& data;
  };

  Handle loadAsync(const char* name, const fastgltf::Asset& asset,
                   const fastgltf::DataSource& data);

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
  bool decRef(Handle handle) { return mData.decRef(handle); }

  Handle getWhite() const { return mWhite; }
  Handle getMissing() const { return mMissing; }

  int getCapacity() const { return mCapacity; }

  const Image& getTexture(Handle handle) { return mData.get(handle); }

private:
  void updateSet(const Image* image, Handle index);
  void resize(int capacity);

  core::HandleList<Image, Handle> mData;

  int mCapacity;

  Handle mWhite;
  Handle mMissing;

  DescriptorAllocator mAllocator;
  vk::DescriptorSetLayout mTextureLayout;
  vk::DescriptorSet mDescriptorSet;
};
} // namespace selwonk::vulkan
