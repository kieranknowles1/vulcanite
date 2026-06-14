#pragma once

#include <cstdint>

#include "material.hpp"
#include "vncore/frustum.hpp"
#include "vncore/handle.hpp"

namespace selwonk::assets {
struct MeshData {
  using Handle = core::Handle<MeshData>;

  static MeshData load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
                       const std::vector<Material>& materials,
                       const Material& defaultMaterial);

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
} // namespace selwonk::assets
