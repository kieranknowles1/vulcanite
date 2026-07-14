#pragma once

#include <vulkan/vulkan.hpp>

#include "image.hpp"
#include "shader.hpp"
#include "vncore/handlelist.hpp"
#include <vnassets/image.hpp>
#include <vncore/cvar.hpp>
#include <vncore/threadpool.hpp>

namespace selwonk::vulkan {

// TODO: Lifetimes
class TextureManager {
public:
  using Handle = assets::ImageBase::Handle;

  struct LoadJob : core::ThreadPool::Job {
    LoadJob(Handle out, const char* name, const fastgltf::Asset& asset,
            const fastgltf::DataSource& data)
        : out(out), name(name), asset(asset), data(data) {}
    void execute() override;
    void finalise() override;

    Handle out;
    const char* name;
    // FIXME: This is not memory safe, the gltf asset may be dropped by now
    // without a full sync
    const fastgltf::Asset& asset;
    const fastgltf::DataSource& data;

    // TODO: Won't be needed once thread safe uploads are a thing
    std::unique_ptr<assets::ImageBase::ImgData> decode;
  };

  struct LoadFileJob : core::ThreadPool::Job {
    LoadFileJob(Handle out, const char* name, core::Vfs::Path path)
      : out(out), name(name), path(path) {}

    void execute() override;
    void finalise() override;

    Handle out;
    const char* name;
    core::Vfs::StrongPath path;

    // TODO: Won't be needed once thread safe uploads are a thing
    std::unique_ptr<assets::ImageBase::ImgData> decode;
  };

  Handle loadAsync(const char* name, const fastgltf::Asset& asset,
                   const fastgltf::DataSource& data);
  Handle loadAsync(const char* name, core::Vfs::Path path);

  size_t size() { return mData.size(); }

  TextureManager(core::Cvar::Int& maxTextures);
  ~TextureManager();

  vk::DescriptorSetLayout getDescriptorLayout() { return mTextureLayout; }
  vk::DescriptorSet getDescriptorSet() { return mDescriptorSet; }

  // Add a texture to the manager. Takes ownership of the original image
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
  // Reserve a slot without uploading anything
  // Using the image without uploading is undefined
  Handle reserve(Image& image) {
    // TODO: Replace image arg with reserve method on store
    return mData.insert(std::move(image));
  }

  // Assign an image to the main image descriptor set
  // TODO: Is this thread safe?
  void writeSet(Handle handle) { updateSet(&mData.get(handle), handle); }

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
