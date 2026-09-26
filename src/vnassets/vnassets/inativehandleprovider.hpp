#pragma once

#include <span>

#include <vncore/singleton.hpp>
#include <vncore/vfs.hpp>

#include "image.hpp"
#include "material.hpp"
#include "sampler.hpp"
#include "mesh.hpp"

namespace selwonk::assets {

#define DECL_COUNTS(Name)                                                      \
  virtual size_t Name##Size() const = 0;                                      \
  virtual size_t Name##Capacity() const = 0;
#define DECL_REFS(HandleType, Name)                                            \
  virtual void incRef(HandleType handle) = 0;                                  \
  virtual bool decRef(HandleType handle) = 0;                                  \
  DECL_COUNTS(Name);

// Main interface for providing native handles to higher-level components and
// interacting with them
class INativeHandleProvider : public core::Singleton<INativeHandleProvider> {
public:
  virtual ~INativeHandleProvider() = default;

#pragma region Samplers
  virtual SamplerConfig::Handle getSampler(SamplerConfig definition) = 0;
  DECL_COUNTS(sampler);
#pragma endregion

#pragma region Textures
  virtual ImageBase::Handle
  loadTextureAsync(const char* name, std::shared_ptr<fastgltf::Asset> asset,
                   const fastgltf::DataSource& data) = 0;
  virtual ImageBase::Handle loadTextureFromFileAsync(const char* name,
                                                     core::Vfs::FilePtr file) = 0;

  virtual ImageBase::Handle getWhite() = 0;

  DECL_REFS(ImageBase::Handle, texture);
#pragma endregion

#pragma region Materials
  virtual Material::DataHandle
  addMaterial(const interop::MaterialData& data) = 0;
  virtual const Material& getDefaultMaterial() = 0;
  DECL_REFS(Material::DataHandle, material);
#pragma endregion

#pragma region Index Buffers
  DECL_REFS(MeshData::IndexHandle, indexBuffer);
  virtual MeshData::IndexHandle addIndexBuffer(std::span<uint32_t> data) = 0;
#pragma endregion

#pragma region Vertex Buffers
  DECL_REFS(MeshData::VertexHandle, vertexBuffer);
  virtual MeshData::VertexHandle addVertexBuffer(std::span<interop::Vertex> data) = 0;
#pragma endregion

#pragma region Meshes
  DECL_REFS(MeshData::Handle, mesh);
  virtual MeshData::Handle addMesh(std::string_view name, MeshData data) = 0;
#pragma endregion

};

} // namespace selwonk::assets
