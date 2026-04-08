#pragma once

#include <vector>

#include "../../assets/shaders/triangle.h"
#include "buffermap.hpp"
#include "fastgltf/types.hpp"
#include "material.hpp"
#include "vncore/handelist.hpp"
#include "vncore/math.hpp"

namespace selwonk::vulkan {
class Mesh {
public:
  struct Bounds {
    glm::vec3 origin;
    float radius;

    const constexpr Bounds operator*(const glm::mat4& transform) const {
      auto scale = core::math::maxScale(transform);
      return Bounds{
          // Don't need to transform origin as it's already been transformed
          // prior to frustum cull
          // .origin = glm::vec3(glm::vec4(origin, 1.0) * transform),
          .origin = origin,
          .radius = radius * scale,
      };
    }
  };

  struct Surface {
    uint32_t mIndexOffset;
    uint32_t mIndexCount;
    Material mMaterial;
  };

  struct Data {
    std::vector<uint32_t> indices;
    std::vector<interop::Vertex> vertices;
    std::vector<Surface> surfaces;
  };

  // A GLTF can contain multiple meshes, each with multiple submeshes
  static core::HandleList<Mesh>::Handle
  load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
       const std::vector<Material>& materials);

  Mesh(std::string_view name, Data data, Bounds bounds);
  ~Mesh();

  // No copy
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
  // No move
  Mesh(Mesh&&) = delete;
  Mesh& operator=(Mesh&&) = default;

  // TODO: Make these private
  // private:
  std::vector<Surface> mSurfaces;
  Bounds mBounds;
  std::string name;

  BufferMap::Handle mIndexBufferIndex;
  BufferMap::Handle mVertexIndex;

  static constexpr std::string_view AttrPosition = "POSITION";
  static constexpr std::string_view AttrNormal = "NORMAL";
  static constexpr std::string_view AttrUv = "TEXCOORD_0";
  static constexpr std::string_view AttrColor = "COLOR_0";
};
} // namespace selwonk::vulkan
