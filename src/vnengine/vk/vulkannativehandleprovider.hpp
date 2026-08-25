#pragma once

#include <vnassets/inativehandleprovider.hpp>

#include "bufferarray.hpp"
#include "samplermanager.hpp"
#include "texturemanager.hpp"
#include "vnassets/image.hpp"
#include "vnassets/material.hpp"
#include "buffermap.hpp"
#include "mesh.hpp"

namespace selwonk::vulkan {

#define IMPL_REFS(HandleType, Container)                                       \
  void incRef(HandleType handle) override { return Container.incRef(handle); } \
  bool decRef(HandleType handle) override { return Container.decRef(handle); }

class VulkanNativeHandleProvider final : public assets::INativeHandleProvider {
public:
  static core::Cvar::Int MaxTextures;
  static core::Cvar::Int MaxVertexBuffers;
  static core::Cvar::Int MaxMaterials;

  VulkanNativeHandleProvider();
  ~VulkanNativeHandleProvider() override;

#pragma region Samplers
  assets::SamplerConfig::Handle
  getSampler(assets::SamplerConfig definition) override {
    return mSamplers.get(definition);
  }

#pragma endregion

#pragma region Textures
  assets::ImageBase::Handle
  loadTextureAsync(const char* name, std::shared_ptr<fastgltf::Asset> asset,
                   const fastgltf::DataSource& data) override {
    return mTextures.loadAsync(name, asset, data);
  }
  assets::ImageBase::Handle
  loadTextureFromFileAsync(const char* name, core::Vfs::Path path) override {
    return mTextures.loadAsync(name, path);
  }

  assets::ImageBase::Handle getWhite() override { return mTextures.getWhite(); }
  IMPL_REFS(assets::ImageBase::Handle, mTextures);

#pragma endregion

#pragma region Materials
  assets::Material::DataHandle
  addMaterial(const interop::MaterialData& data) override {
    return mMaterials.insert(data);
  }
  const assets::Material& getDefaultMaterial() override { return mDefaultMaterial; }
  IMPL_REFS(assets::Material::DataHandle, mMaterials);
#pragma endregion

#pragma region Index Buffers
  assets::MeshData::IndexHandle addIndexBuffer(std::span<uint32_t> data) override {
    return mIndexBuffers.insert(data, Buffer::Usage::BindlessIndex);
  }
  IMPL_REFS(assets::MeshData::IndexHandle, mIndexBuffers);
#pragma endregion

#pragma region Vertex Buffers
  assets::MeshData::VertexHandle addVertexBuffer(std::span<interop::Vertex> data) override {
    return mVertexBuffers.insert(data, Buffer::Usage::BindlessVertex);
  }
  IMPL_REFS(assets::MeshData::VertexHandle, mVertexBuffers);
#pragma endregion

#pragma region Meshes
  assets::MeshData::Handle addMesh(std::string_view name, assets::MeshData data) override {
    return mMeshes.insert(name, data);
  }
  IMPL_REFS(assets::MeshData::Handle, mMeshes);
#pragma endregion


  SamplerManager& getNativeSamplers() { return mSamplers; }
  TextureManager& getNativeTextures() { return mTextures; }
  BufferArray<interop::MaterialData>& getNativeMaterials() {
    return mMaterials;
  }
  BufferMap<assets::MeshData::IndexHandle>& getNativeIndexes() { return mIndexBuffers; }
  BufferMap<assets::MeshData::VertexHandle>& getNativeVertexes() { return mVertexBuffers; }
  core::HandleList<Mesh, assets::MeshData::Handle>& getNativeMeshes() { return mMeshes; }

private:
  SamplerManager mSamplers;
  TextureManager mTextures;
  BufferArray<interop::MaterialData> mMaterials;
  BufferMap<assets::MeshData::IndexHandle> mIndexBuffers;
  BufferMap<assets::MeshData::VertexHandle> mVertexBuffers;
  core::HandleList<Mesh, assets::MeshData::Handle> mMeshes;

  assets::Material mDefaultMaterial;
};

#undef IMPL_REFS

} // namespace selwonk::vulkan
