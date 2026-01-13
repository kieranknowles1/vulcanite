#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../../assets/shaders/triangle.h"
#include "buffer.hpp"
#include "fastgltf/types.hpp"
#include "material.hpp"

namespace selwonk::vulkan {
class Mesh {
public:
  struct Bounds {
    glm::vec3 origin;
    float radius;
  };

  struct Surface {
    uint32_t mIndexOffset;
    uint32_t mIndexCount;
    std::shared_ptr<Material> mMaterial;
  };

  struct Data {
    std::vector<uint32_t> indices;
    std::vector<interop::Vertex> vertices;
    std::vector<Surface> surfaces;
  };

  // A GLTF can contain multiple meshes, each with multiple submeshes
  static std::unique_ptr<Mesh>
  load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
       const std::vector<std::shared_ptr<Material>>& materials);

  Mesh(std::string_view name, Data data, Bounds bounds);
  ~Mesh();

  // No copy
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  // TODO: Make these private
  // private:
  std::vector<Surface> mSurfaces;
  Bounds mBounds;
  std::string name;
  size_t mIndexCount;

  Buffer mIndexBuffer;
  uint32_t mVertexIndex;
  Buffer mVertexBuffer;

  static constexpr std::string_view AttrPosition = "POSITION";
  static constexpr std::string_view AttrNormal = "NORMAL";
  static constexpr std::string_view AttrUv = "TEXCOORD_0";
  static constexpr std::string_view AttrColor = "COLOR_0";
};
} // namespace selwonk::vulkan
