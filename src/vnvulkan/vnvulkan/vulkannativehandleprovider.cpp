#include "vulkannativehandleprovider.hpp"

#include <backends/imgui_impl_vulkan.h>

#include <vncore/cvar.hpp>

namespace selwonk::vulkan {

core::Cvar::Int VulkanNativeHandleProvider::MaxTextures("render.max_textures", 8192,
  "Maximum number of textures",
  core::Cvar::Flags::Unsigned);

core::Cvar::Int VulkanNativeHandleProvider::MaxVertexBuffers("render.max_vertex_buffers", 8192,
  "Maximum number of vertex buffers",
  core::Cvar::Flags::Unsigned);

core::Cvar::Int VulkanNativeHandleProvider::MaxMaterials("render.max_materials", 8192,
  "Maximum number of materials",
  core::Cvar::Flags::Unsigned);


VulkanNativeHandleProvider::VulkanNativeHandleProvider(core::ThreadPool& threadPool) 
  : mTextures(threadPool, MaxTextures), mIndexBuffers(MaxVertexBuffers), mVertexBuffers(MaxVertexBuffers) {
  // TODO: RAII
  mMaterials.init(MaxMaterials);


  interop::MaterialData defaultMat = {
    .colorFactors = glm::vec4(1.0f),
    .metalRoughnessFactors = glm::vec4(1.0f),
  };
  mDefaultMaterial = assets::Material{
    .mTexture = mTextures.getMissing(),
    .mDataIndex = mMaterials.insert(defaultMat),
    .mSampler = getSampler({
        .mMinFilter = fastgltf::Filter::Nearest,
        .mMagFilter = fastgltf::Filter::Nearest,
    }),
    .mPass = assets::Material::Pass::Opaque,
  };
}

VulkanNativeHandleProvider::~VulkanNativeHandleProvider() {
  mMaterials.decRef(mDefaultMaterial.mDataIndex);
}

ImTextureID VulkanNativeHandleProvider::registerGuiTexture(assets::ImageBase::Handle texture)
{
  auto native = getNativeTextures().getTexture(texture).getView();
  auto id = ImGui_ImplVulkan_AddTexture(native, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  static_assert(sizeof(decltype(id)) <= sizeof(ImTextureID));
  return reinterpret_cast<ImTextureID>(id);
}

void VulkanNativeHandleProvider::freeGuiTexture(ImTextureID id)
{
  ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(id));
}

}