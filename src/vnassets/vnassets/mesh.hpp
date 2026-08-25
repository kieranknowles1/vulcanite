#pragma once

#include <cstdint>

#include "material.hpp"
#include "vncore/frustum.hpp"
#include "vncore/handle.hpp"

namespace selwonk::assets {
// TODO: Merge this with main mesh class
struct MeshData {
  using Handle = core::Handle<MeshData>;

  using VertexHandle = core::Handle<interop::Vertex>;
  using IndexHandle = core::Handle<uint32_t>;

  // Decode a GLTF asset. Does not increment ref counts
  static MeshData load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
                       const std::vector<Material>& materials);

  static constexpr std::string_view AttrPosition = "POSITION";
  static constexpr std::string_view AttrNormal = "NORMAL";
  static constexpr std::string_view AttrUv = "TEXCOORD_0";
  static constexpr std::string_view AttrColor = "COLOR_0";

  struct Surface {
    uint32_t mIndexOffset;
    uint32_t mIndexCount;
    Material mMaterial;
  };

  std::vector<uint32_t> indices;
  std::vector<interop::Vertex> vertices;
  std::vector<Surface> surfaces;
  core::Bounds bounds;
};

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
} // namespace selwonk::assets
