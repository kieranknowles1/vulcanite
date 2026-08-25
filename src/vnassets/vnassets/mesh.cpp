#include "mesh.hpp"

#include <fastgltf/glm_element_traits.hpp>

#include "inativehandleprovider.hpp"

namespace selwonk::assets {
MeshData MeshData::load(const fastgltf::Asset& asset,
                        const fastgltf::Mesh& mesh,
                        const std::vector<Material>& materials) {
  auto& interop = INativeHandleProvider::get();
  MeshData data;

  for (auto& primitive : mesh.primitives) {
    auto& indices = asset.accessors[primitive.indicesAccessor.value()];

    Surface surface;
    surface.mIndexOffset = data.indices.size();
    surface.mIndexCount = indices.count;
    int startVertex = data.vertices.size();
    if (primitive.materialIndex.has_value())
      surface.mMaterial = materials[primitive.materialIndex.value()];
    else
      surface.mMaterial = interop.getDefaultMaterial();
    data.surfaces.push_back(surface);

    auto& positions =
        asset.accessors[primitive.findAttribute(AttrPosition)->accessorIndex];
    fastgltf::iterateAccessor<glm::vec3>(asset, positions, [&](auto&& pos) {
      data.vertices.push_back({.position = pos});
    });

    fastgltf::iterateAccessor<uint32_t>(asset, indices, [&](uint32_t idx) {
      data.indices.push_back(idx + startVertex);
      assert(data.indices.back() < data.vertices.size() &&
             "Index out of bounds, undefined behaviour or read from different "
             "submesh");
    });

#define UPSERT_ATTR(name, field, type)                                         \
  {                                                                            \
    auto attr = primitive.findAttribute(name);                                 \
    if (attr != primitive.attributes.end()) {                                  \
      auto& access = asset.accessors[attr->accessorIndex];                     \
      fastgltf::iterateAccessorWithIndex<type>(                                \
          asset, access, [&](auto&& value, size_t index) {                     \
            data.vertices[startVertex + index].field = value;                  \
          });                                                                  \
    }                                                                          \
  }
    auto uvs = primitive.findAttribute(AttrUv);
    if (uvs != primitive.attributes.end()) {
      auto& access = asset.accessors[uvs->accessorIndex];
      fastgltf::iterateAccessorWithIndex<glm::vec2>(
          asset, access, [&](auto&& value, size_t index) {
            data.vertices[startVertex + index].uvX = value.x;
            data.vertices[startVertex + index].uvY = value.y;
          });
    }

    UPSERT_ATTR(AttrNormal, normal, glm::vec3)
    UPSERT_ATTR(AttrColor, color, glm::vec4)

    if (primitive.findAttribute(AttrColor) == primitive.attributes.end()) {
      for (auto& vtx : data.vertices) {
        vtx.color = glm::vec4(1.0f);
      }
    }
  }
  glm::vec3 min = data.vertices[0].position;
  glm::vec3 max = data.vertices[0].position;
  for (auto& vtx : data.vertices) {
    min = glm::min(min, vtx.position);
    max = glm::max(max, vtx.position);
  }
  data.bounds.origin = (min + max) / 2.0f;
  data.bounds.radius = glm::length(min - max) / 2.0f;
  return data;
}

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

} // namespace selwonk::assets
