#include "texturemanager.hpp"

#include <array>
#include <fmt/base.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

TextureManager::TextureManager() {
  std::array<DescriptorAllocator::PoolSizeRatio, 1> sizes = {
      {{vk::DescriptorType::eSampledImage, 1}}};
  mAllocator.init(MaxTextures, sizes);

  DescriptorLayoutBuilder builder;
  builder.addBinding(0, vk::DescriptorType::eSampledImage, MaxTextures);
  mTextureLayout = builder.build(VulkanHandle::get().mDevice,
                                 vk::ShaderStageFlagBits::eFragment);
  mDescriptorSet = mAllocator.allocate<ImageDescriptor>(mTextureLayout);

  // Create default textures for use elsewhere
  const vk::Format format = vk::Format::eR8G8B8A8Unorm;
  const auto oneByOne = vk::Extent3D(1, 1, 1);
  const auto usage = vk::ImageUsageFlagBits::eSampled |
                     vk::ImageUsageFlagBits::eTransferDst |
                     vk::ImageUsageFlagBits::eStorage;
  const auto white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
  const auto black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
  const auto magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));

  auto whiteTex = std::make_unique<Image>(oneByOne, format, usage);
  whiteTex->fill(&white, sizeof(white));
  mWhite = insert(std::move(whiteTex));

  // Source engine missing texture or no missing texture
  const int missingTextureSize = 16;
  auto missingTexture = std::make_unique<Image>(
      vk::Extent3D{missingTextureSize, missingTextureSize, 1}, format, usage);
  std::array<uint32_t, missingTextureSize * missingTextureSize>
      missingTextureData;
  for (int x = 0; x < missingTextureSize; ++x) {
    for (int y = 0; y < missingTextureSize; ++y) {
      // Alternate color
      auto color = (x + y) % 2 == 0 ? magenta : black;
      missingTextureData[x + y * missingTextureSize] = color;
    }
  }
  missingTexture->fill(missingTextureData);
  mMissing = insert(std::move(missingTexture));
}

TextureManager::~TextureManager() {
  auto& handle = VulkanHandle::get();
  handle.mDevice.destroyDescriptorSetLayout(mTextureLayout, nullptr);
  mAllocator.destroy();
}

std::unique_ptr<Image>
TextureManager::create(const std::filesystem::path& params, Handle index) {
  if (index.value() >= MaxTextures)
    throw std::runtime_error("Too many textures");

  assert(false && "Not implemented");
}

void TextureManager::updateSet(const Image* image, Handle index) {
  if (!mZeroed) {
    mZeroed = true;
    // Zero all slots as required for Vulcan to not complain
    for (int i = 0; i < MaxTextures; i++) {
      updateSet(image, Handle(i));
    }
  }

  mDescriptorSet.write(VulkanHandle::get().mDevice,
                       {image->getView(), vk::DescriptorType::eSampledImage,
                        vk::ImageLayout::eShaderReadOnlyOptimal,
                        index.value()});
}

} // namespace selwonk::vulkan
