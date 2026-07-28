#pragma once

#include <vector>

#include "buffermap.hpp"
#include "fastgltf/types.hpp"
#include "vncore/frustum.hpp"

#include <vnassets/material.hpp>
#include <vnassets/mesh.hpp>

namespace selwonk::vulkan {
class Mesh {
public:
  // A GLTF can contain multiple meshes, each with multiple submeshes
  static assets::MeshData::Handle
  load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
       const std::vector<assets::Material>& materials);

  Mesh(std::string_view name, assets::MeshData data);
  ~Mesh();

  // No copy
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
  // No move
  Mesh(Mesh&&) = delete;
  Mesh& operator=(Mesh&&) = default;

  // TODO: Make these private
  // TODO: Maybe retain all MeshData after load
  // private:
  std::vector<assets::MeshData::Surface> mSurfaces;
  core::Bounds mBounds;
  std::string name;

  assets::MeshData::IndexHandle mIndexBufferIndex;
  assets::MeshData::VertexHandle mVertexIndex;
};
} // namespace selwonk::vulkan
