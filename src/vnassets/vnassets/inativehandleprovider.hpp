#pragma once

#include <span>

#include <vncore/singleton.hpp>
#include <vncore/vfs.hpp>

#include "image.hpp"
#include "material.hpp"
#include "sampler.hpp"
#include "mesh.hpp"

namespace selwonk::assets {

#define DECL_REFS(HandleType)                                                  \
  virtual void incRef(HandleType handle) = 0;                                  \
  virtual bool decRef(HandleType handle) = 0;

// Main interface for providing native handles to higher-level components and
// interacting with them
class INativeHandleProvider : public core::Singleton<INativeHandleProvider> {
public:
  virtual ~INativeHandleProvider() = default;

#pragma region Samplers
  virtual SamplerConfig::Handle getSampler(SamplerConfig definition) = 0;
#pragma endregion

#pragma region Textures
  virtual ImageBase::Handle
  loadTextureAsync(const char* name, std::shared_ptr<fastgltf::Asset> asset,
                   const fastgltf::DataSource& data) = 0;
  virtual ImageBase::Handle loadTextureFromFileAsync(const char* name,
                                                     core::Vfs::Path path) = 0;

  virtual ImageBase::Handle getWhite() = 0;

  DECL_REFS(ImageBase::Handle);
#pragma endregion

#pragma region Materials
  virtual Material::DataHandle
  addMaterial(const interop::MaterialData& data) = 0;
  virtual const Material& getDefaultMaterial() = 0;
  DECL_REFS(Material::DataHandle);
#pragma endregion

#pragma region Index Buffers
  DECL_REFS(MeshData::IndexHandle);
  virtual MeshData::IndexHandle addIndexBuffer(std::span<uint32_t> data) = 0;
#pragma endregion

#pragma region Vertex Buffers
  DECL_REFS(MeshData::VertexHandle);
  virtual MeshData::VertexHandle addVertexBuffer(std::span<interop::Vertex> data) = 0;
#pragma endregion

#pragma region Meshes
  DECL_REFS(MeshData::Handle);
  virtual MeshData::Handle addMesh(std::string_view name, MeshData data) = 0;
#pragma endregion

};

} // namespace selwonk::assets
