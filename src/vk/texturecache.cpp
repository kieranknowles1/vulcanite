#include "texturecache.hpp"

#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <array>
#include <fmt/base.h>

namespace selwonk::vulkan {

TextureCache::TextureCache() {
  std::array<DescriptorAllocator::PoolSizeRatio, 1> sizes = {
      {{vk::DescriptorType::eSampledImage, 1}}};
  mAllocator.init(MaxTextures, sizes);

  DescriptorLayoutBuilder builder;
  builder.addBinding(0, vk::DescriptorType::eSampledImage, MaxTextures);
  mTextureLayout = builder.build(VulkanHandle::get().mDevice,
                                 vk::ShaderStageFlagBits::eFragment);
  mDescriptorSet = mAllocator.allocate<ImageDescriptor>(mTextureLayout);
}

TextureCache::~TextureCache() {
  auto& handle = VulkanHandle::get();
  handle.mDevice.destroyDescriptorSetLayout(mTextureLayout, nullptr);
  mAllocator.destroy();
}

std::unique_ptr<Image> TextureCache::create(const TextureKey& params,
                                            Handle index) {
  if (index.value() >= MaxTextures)
    throw std::runtime_error("Too many textures");

  std::visit(
      [](const auto& key) {
        using T = std::decay_t<decltype(key)>;
        if constexpr (std::is_same_v<T, std::filesystem::path>) {
          assert(false && "Not implemented");
        } else if constexpr (std::is_same_v<T, GltfImage>) {
          assert(false && "GLTF textures must be inserted manually");
        } else {
          static_assert(false, "Non-exhaustive visitor");
        }
      },
      params);
  assert(false && "Not implemented");
}

void TextureCache::updateSet(const Image* image, Handle index) {
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
