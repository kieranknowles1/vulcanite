#include "mesh.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "buffer.hpp"
#include "vnassets/mesh.hpp"
#include "vulkanengine.hpp"

namespace selwonk::vulkan {

assets::MeshData::Handle
Mesh::load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
           const std::vector<assets::Material>& materials) {
  auto& engine = VulkanEngine::get();
  auto data = assets::MeshData::load(asset, mesh, materials,
                                     engine.getDefaultMaterial());
  return engine.mMeshes.insert(mesh.name, std::move(data));
}

Mesh::Mesh(std::string_view name, assets::MeshData data)
    : mSurfaces(std::move(data.surfaces)), mBounds(data.bounds), name(name) {
  auto& engine = VulkanEngine::get();
  // Increments ref
  mIndexBufferIndex = engine.getIndexBuffers().insert(
      std::span(data.indices), Buffer::Usage::BindlessIndex);
  // Increments ref
  mVertexIndex = engine.getVertexBuffers().insert(
      std::span(data.vertices), Buffer::Usage::BindlessVertex);

  for (auto& surface : mSurfaces) {
    engine.getTextureManager().incRef(surface.mMaterial.mTexture);
    engine.mMaterials.incRef(surface.mMaterial.mDataIndex);
  }
}

Mesh::~Mesh() {
  auto& engine = VulkanEngine::get();
  engine.getVertexBuffers().decRef(mVertexIndex);
  engine.getIndexBuffers().decRef(mIndexBufferIndex);
  for (auto& surface : mSurfaces) {
    engine.getTextureManager().decRef(surface.mMaterial.mTexture);
    engine.mMaterials.decRef(surface.mMaterial.mDataIndex);
  }
}

} // namespace selwonk::vulkan
