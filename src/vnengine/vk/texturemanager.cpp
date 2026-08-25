#include "texturemanager.hpp"

#include <array>
#include <fmt/base.h>
#include <glm/glm.hpp>
#include <memory>

#include "fastgltf/types.hpp"
#include "shader.hpp"
#include "vnassets/image.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

TextureManager::TextureManager(core::Cvar::Int& maxTextures)
    : mCapacity(maxTextures.value()) {
  resize(mCapacity);
  maxTextures.getStore().addChange([this](int capacity) { resize(capacity); });
  maxTextures.getStore().addValidate(
      [this](int capacity) -> std::optional<std::string> {
        if (capacity < mData.size())
          return "Cannot be smaller than current size (" +
                 std::to_string(mData.size()) + ")";
        return std::nullopt;
      });

  // Create default textures for use elsewhere
  const vk::Format format = vk::Format::eR8G8B8A8Unorm;
  const auto oneByOne = vk::Extent3D(1, 1, 1);
  const auto usage = vk::ImageUsageFlagBits::eSampled |
                     vk::ImageUsageFlagBits::eTransferDst |
                     vk::ImageUsageFlagBits::eStorage;
  const auto white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
  const auto black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
  const auto magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));

  Image whiteTex(oneByOne, format, usage, "TexWhite");
  whiteTex.fill(&white, sizeof(white));
  mWhite = insert(whiteTex);

  // Source engine missing texture or no missing texture
  const int missingTextureSize = 16;
  std::array<uint32_t, missingTextureSize * missingTextureSize>
      missingTextureData;
  for (int x = 0; x < missingTextureSize; ++x) {
    for (int y = 0; y < missingTextureSize; ++y) {
      // Alternate color
      auto color = (x + y) % 2 == 0 ? magenta : black;
      missingTextureData[x + y * missingTextureSize] = color;
    }
  }
  Image missingTexture(vk::Extent3D{missingTextureSize, missingTextureSize, 1},
                       format, usage, "TexMissing");
  missingTexture.fill(missingTextureData);
  mMissing = insert(missingTexture);
}

void TextureManager::LoadJob::execute() {
  auto& manager = VulkanEngine::get().getNativeHandles().getNativeTextures();

  // TODO: Error handling in threads
  decode = std::make_unique<assets::ImageBase::ImgData>(
      assets::ImageBase::ImgData::loadFromAsset(*asset, data));
  auto& outimg = manager.mData.get(out);
  // TODO: Could we load/upload fewer channels if the image has fewer?
  // TODO: Helper to resize from imgdata
  outimg.allocate(
      {decode->width, decode->height, 1}, vk::Format::eR8G8B8A8Unorm,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
      name);
}

void TextureManager::LoadJob::finalise() {
  auto& manager = VulkanEngine::get().getNativeHandles().getNativeTextures();

  auto& outimg = manager.mData.get(out);
  // TODO: ImmediateSubmit is not thread safe
  outimg.fill(decode->data, decode->width * decode->height * 4);
  // TODO: This may be thread safe, but depends on submit
  manager.writeSet(out);

  if (manager.decRef(out)) {
    SPDLOG_WARN("Texture {} loaded but handle not referenced elsewhere", name);
  }
}

void TextureManager::LoadFileJob::execute() {
  auto& engine = VulkanEngine::get();
  auto& manager = engine.getNativeHandles().getNativeTextures();

  std::vector<char> data;
  engine.getVfs().get(path)->readfull(data);
  decode = std::make_unique<assets::ImageBase::ImgData>(
    assets::ImageBase::ImgData::loadFromMemory(reinterpret_cast<std::byte*>(data.data()), data.size()));
  auto& outimg = manager.mData.get(out);

  // TODO: Could we load/upload fewer channels if the image has fewer?
  // TODO: Helper to resize from imgdata
  outimg.allocate(
    { decode->width, decode->height, 1 }, vk::Format::eR8G8B8A8Unorm,
    vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
    name);
}

void TextureManager::LoadFileJob::finalise() {
  // TODO: Common finalise function
  auto& manager = VulkanEngine::get().getNativeHandles().getNativeTextures();

  auto& outimg = manager.mData.get(out);
  // TODO: ImmediateSubmit is not thread safe
  outimg.fill(decode->data, decode->width * decode->height * 4);
  // TODO: This may be thread safe, but depends on submit
  manager.writeSet(out);

  if (manager.decRef(out)) {
    SPDLOG_WARN("Texture {} loaded but handle not referenced elsewhere", name);
  }
}

TextureManager::Handle
TextureManager::loadAsync(const char* name, std::shared_ptr<fastgltf::Asset> asset,
                          const fastgltf::DataSource& data) {
  auto& engine = VulkanEngine::get();
  auto& threadPool = engine.getThreadPool();

  Image image; // TODO: Don't create an image yet
  auto handle = reserve(image);
  incRef(handle); // Job owns its handle
  threadPool.addJob(std::make_unique<LoadJob>(handle, name, asset, data));
  return handle;
}

TextureManager::Handle
TextureManager::loadAsync(const char* name, core::Vfs::Path path) {
  auto& engine = VulkanEngine::get();
  auto& threadPool = engine.getThreadPool();

  Image image; // TODO: Don't create an image yet
  auto handle = reserve(image);
  incRef(handle); // Job owns its handle
  threadPool.addJob(std::make_unique<LoadFileJob>(handle, name, path));
  return handle;
}

void TextureManager::resize(int capacity) {
  mCapacity = capacity;
  // TODO: Free descriptors once the current frame is finished
  std::array<DescriptorAllocator::PoolSizeRatio, 1> sizes = {
      {{vk::DescriptorType::eSampledImage, 1}}};
  mAllocator.init(capacity, sizes);

  DescriptorLayoutBuilder builder;
  builder.addBinding(0, vk::DescriptorType::eSampledImage, capacity);
  mTextureLayout = builder.build(VulkanHandle::get().mDevice,
                                 vk::ShaderStageFlagBits::eFragment);
  mDescriptorSet = mAllocator.allocate(mTextureLayout);

  for (int i = 0; i < mData.maxId(); i++) {
    updateSet(&mData.getUnsafeNoGeneration(i), Handle(i, 0));
  }
}

TextureManager::~TextureManager() {
  mData.decRef(mWhite);
  mData.decRef(mMissing);

  auto& handle = VulkanHandle::get();
  handle.mDevice.destroyDescriptorSetLayout(mTextureLayout, nullptr);
  mAllocator.destroy();
}

void TextureManager::updateSet(const Image* image, Handle index) {
  DescriptorAllocator::writeImage(mDescriptorSet, image->getView(),
                                  index.value(),
                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                  vk::DescriptorType::eSampledImage);
}

} // namespace selwonk::vulkan
