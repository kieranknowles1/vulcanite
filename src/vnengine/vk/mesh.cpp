#include "mesh.hpp"

#include "vnassets/mesh.hpp"
#include "vulkanengine.hpp"

namespace selwonk::vulkan {

assets::MeshData::Handle
Mesh::load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
           const std::vector<assets::Material>& materials) {
  auto& interop = assets::INativeHandleProvider::get();
  auto data = assets::MeshData::load(asset, mesh, materials);
  return interop.addMesh(mesh.name, std::move(data));
}

Mesh::Mesh(std::string_view name, assets::MeshData data)
    : mSurfaces(std::move(data.surfaces)), mBounds(data.bounds), name(name) {
  auto& interop = assets::INativeHandleProvider::get();

  // Increments ref
  mIndexBufferIndex = interop.addIndexBuffer(std::span(data.indices));
  // Increments ref
  mVertexIndex = interop.addVertexBuffer(std::span(data.vertices));

  for (auto& surface : mSurfaces) {
    interop.incRef(surface.mMaterial.mTexture);
    interop.incRef(surface.mMaterial.mDataIndex);
  }
}

Mesh::~Mesh() {
  auto& interop = assets::INativeHandleProvider::get();
  interop.decRef(mVertexIndex);
  interop.decRef(mIndexBufferIndex);

  for (auto& surface : mSurfaces) {
    interop.decRef(surface.mMaterial.mTexture);
    interop.decRef(surface.mMaterial.mDataIndex);
  }
}

} // namespace selwonk::vulkan
