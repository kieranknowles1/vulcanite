#include "texturemanager.hpp"

#include <array>
#include <fmt/base.h>
#include <glm/glm.hpp>

#include "fastgltf/types.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"
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

TextureManager::Handle
TextureManager::loadAsync(const char* name, const fastgltf::Asset& asset,
                          const fastgltf::DataSource& data) {
  try {
    auto decode = assets::ImageBase::ImgData::loadFromAsset(asset, data);
    auto image = Image::upload(name, decode);
    return insert(image);
  } catch (std::runtime_error e) {
    SPDLOG_ERROR("Failed to load image {}", name);
    return getMissing();
  }
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
    // TODO: Generations
    updateSet(&mData.get(Handle(i, 0)), Handle(i, 0));
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
