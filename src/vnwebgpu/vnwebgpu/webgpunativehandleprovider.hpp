#pragma once

#include <vnassets/inativehandleprovider.hpp>

namespace selwonk::webgpu {

#define IMPL_COUNTS(Container, Name)                                           \
  size_t Name##Size() const override { return Container.size(); }            \
  size_t Name##Capacity() const override { return Container.capacity(); };
#define IMPL_REFS(HandleType, Container, Name)                                 \
  void incRef(HandleType handle) override { return Container.incRef(handle); } \
  bool decRef(HandleType handle) override { return Container.decRef(handle); } \
  IMPL_COUNTS(Container, Name);

// TODO: Implement
#define DUMMY override { assert(false); }
#define DUMMY_COUNTS(Name) size_t Name##Size() const DUMMY size_t Name##Capacity() const DUMMY
#define DUMMY_REFS(HandleType, Name) DUMMY_COUNTS(Name) void incRef(HandleType) DUMMY bool decRef(HandleType) DUMMY

class WebGpuNativeHandleProvider final : public assets::INativeHandleProvider {
public:
  WebGpuNativeHandleProvider() = default;
  ~WebGpuNativeHandleProvider() override = default;

#pragma region Samplers
  assets::SamplerConfig::Handle
  getSampler(assets::SamplerConfig definition) DUMMY
  DUMMY_COUNTS(sampler);

#pragma endregion

#pragma region Textures
  assets::ImageBase::Handle
  loadTextureAsync(const char* name, std::shared_ptr<fastgltf::Asset> asset,
                   const fastgltf::DataSource& data) DUMMY
  assets::ImageBase::Handle
  loadTextureFromFileAsync(const char* name, core::Vfs::FilePtr file) DUMMY

  assets::ImageBase::Handle getWhite() DUMMY
  DUMMY_REFS(assets::ImageBase::Handle, texture);

#pragma endregion

#pragma region Materials
  assets::Material::DataHandle
  addMaterial(const interop::MaterialData& data) DUMMY
  const assets::Material& getDefaultMaterial() DUMMY
  DUMMY_REFS(assets::Material::DataHandle, material);
#pragma endregion

#pragma region Index Buffers
  assets::MeshData::IndexHandle addIndexBuffer(std::span<uint32_t> data) DUMMY
  DUMMY_REFS(assets::MeshData::IndexHandle, indexBuffer);
#pragma endregion

#pragma region Vertex Buffers
  assets::MeshData::VertexHandle addVertexBuffer(std::span<interop::Vertex> data) DUMMY
  DUMMY_REFS(assets::MeshData::VertexHandle, vertexBuffer);
#pragma endregion

#pragma region Meshes
  assets::MeshData::Handle addMesh(std::string_view name, assets::MeshData data) DUMMY
  DUMMY_REFS(assets::MeshData::Handle, mesh);
#pragma endregion

protected:
  ImTextureID registerGuiTexture(assets::ImageBase::Handle texture) DUMMY;
  void freeGuiTexture(ImTextureID id) DUMMY;
};

#undef IMPL_REFS

} // namespace selwonk::vulkan
