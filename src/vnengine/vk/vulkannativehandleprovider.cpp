#include "vulkannativehandleprovider.hpp"

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


VulkanNativeHandleProvider::VulkanNativeHandleProvider() 
  : mTextures(MaxTextures), mIndexBuffers(MaxVertexBuffers), mVertexBuffers(MaxVertexBuffers) {
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

}