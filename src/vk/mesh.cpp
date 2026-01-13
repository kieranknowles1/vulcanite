#include "mesh.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "buffer.hpp"
#include "fastgltf/tools.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

std::unique_ptr<Mesh>
Mesh::load(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh,
           const std::vector<std::shared_ptr<Material>>& materials) {
  Data data;
  for (auto& primitive : mesh.primitives) {
    auto& indices = asset.accessors[primitive.indicesAccessor.value()];

    Mesh::Surface surface;
    surface.mIndexOffset = data.indices.size();
    surface.mIndexCount = indices.count;
    int startVertex = data.vertices.size();
    if (primitive.materialIndex.has_value())
      surface.mMaterial = materials[primitive.materialIndex.value()];
    else
      surface.mMaterial = VulkanEngine::get().mDefaultMaterial;
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
  Bounds bounds;
  bounds.origin = (min + max) / 2.0f;
  bounds.radius = glm::length(min - max) / 2.0f;

  return std::make_unique<Mesh>(mesh.name, std::move(data), bounds);
}

Mesh::Mesh(std::string_view name, Data data, Bounds bounds)
    : mSurfaces(std::move(data.surfaces)), mBounds(bounds), name(name),
      mIndexCount(data.indices.size()) {
  const auto indexSize = data.indices.size() * sizeof(uint32_t);
  const auto vertexSize = data.vertices.size() * sizeof(interop::Vertex);

  mIndexBuffer.allocate(indexSize, Buffer::Usage::BindlessIndex);
  mVertexBuffer.allocate(vertexSize, Buffer::Usage::BindlessVertex);

  mIndexBuffer.uploadToGpu(data.indices.data(), indexSize);
  mVertexBuffer.uploadToGpu(data.vertices.data(), vertexSize);

  // TODO: Proper class for this
  auto& engine = VulkanEngine::get();
  mVertexIndex = engine.nextVertexBuffer;
  engine.nextVertexBuffer++;

  vk::DescriptorBufferInfo info = {
      .buffer = mVertexBuffer.getBuffer(),
      .offset = 0,
      .range = vertexSize,
  };
  vk::WriteDescriptorSet write = {
      .dstSet = engine.mBufferSet,
      .dstBinding = 0,
      .dstArrayElement = mVertexIndex,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .pBufferInfo = &info,
  };
  engine.mHandle.mDevice.updateDescriptorSets(1, &write, 0, nullptr);
}

Mesh::~Mesh() {
  auto& handle = VulkanHandle::get();
  mVertexBuffer.free(handle.mAllocator);
  mIndexBuffer.free(handle.mAllocator);
}

} // namespace selwonk::vulkan
