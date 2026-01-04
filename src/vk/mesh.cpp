#include "mesh.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "buffer.hpp"
#include "fastgltf/tools.hpp"
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
    UPSERT_ATTR(AttrNormal, normal, glm::vec3)
    UPSERT_ATTR(AttrUv, uv, glm::vec2)
    UPSERT_ATTR(AttrColor, color, glm::vec4)

    if (primitive.findAttribute(AttrColor) == primitive.attributes.end()) {
      for (auto& vtx : data.vertices) {
        vtx.color = glm::vec4(1.0f);
      }
    }
  }
  return std::make_unique<Mesh>(mesh.name, std::move(data));
}

Mesh::Mesh(std::string_view name, Data data)
    : mSurfaces(std::move(data.surfaces)), name(name),
      mIndexCount(data.indices.size()) {
  auto& handle = VulkanHandle::get();
  const auto indexSize = data.indices.size() * sizeof(uint32_t);
  const auto vertexSize = data.vertices.size() * sizeof(interop::Vertex);

  // Add eTransferDst to both buffers so we can upload to them
  mIndexBuffer.allocate(handle.mAllocator, indexSize,
                        vk::BufferUsageFlagBits::eIndexBuffer |
                            vk::BufferUsageFlagBits::eTransferDst,
                        VMA_MEMORY_USAGE_GPU_ONLY);
  mVertexBuffer.allocate(handle.mAllocator, vertexSize,
                         vk::BufferUsageFlagBits::eShaderDeviceAddress |
                             vk::BufferUsageFlagBits::eTransferDst,
                         VMA_MEMORY_USAGE_GPU_ONLY);

  // TODO: Should we reuse the staging buffer?
  auto stagingBuffer =
      Buffer::transferBuffer(handle.mAllocator, vertexSize + indexSize);
  void* gpuData = stagingBuffer.getAllocationInfo().pMappedData;
  assert(gpuData != nullptr);
  memcpy(gpuData, data.vertices.data(), vertexSize);
  memcpy(reinterpret_cast<uint8_t*>(gpuData) + vertexSize, data.indices.data(),
         indexSize);

  handle.immediateSubmit([&](vk::CommandBuffer cmd) {
    vk::BufferCopy vtxCopy = {
        .srcOffset = 0, .dstOffset = 0, .size = vertexSize};
    vk::BufferCopy idxCopy = {
        .srcOffset = vertexSize, .dstOffset = 0, .size = indexSize};
    cmd.copyBuffer(stagingBuffer.getBuffer(), mVertexBuffer.getBuffer(), 1,
                   &vtxCopy);
    cmd.copyBuffer(stagingBuffer.getBuffer(), mIndexBuffer.getBuffer(), 1,
                   &idxCopy);
  });
  stagingBuffer.free(handle.mAllocator);
}

Mesh::~Mesh() {
  auto& handle = VulkanHandle::get();
  mVertexBuffer.free(handle.mAllocator);
  mIndexBuffer.free(handle.mAllocator);
}

} // namespace selwonk::vulkan
