#include "mesh.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "buffer.hpp"
#include "fastgltf/tools.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

std::vector<std::shared_ptr<Mesh>> Mesh::load(Vfs::SubdirPath path) {
  auto &vfs = VulkanEngine::get().getVfs();

  std::vector<std::byte> buffer;
  vfs.readfull(Vfs::Meshes / path, buffer);

  auto data = fastgltf::GltfDataBuffer::FromBytes(buffer.data(), buffer.size());
  if (data.error() != fastgltf::Error::None) {
    throw LoadException(data.error());
  }

  fastgltf::Parser parser;
  auto load = parser.loadGltf(data.get(), "/");
  if (load.error() != fastgltf::Error::None) {
    throw LoadException(load.error());
  }

  auto asset = std::move(load.get());

  std::vector<std::shared_ptr<Mesh>> out;
  for (auto &gmesh : asset.meshes) {
    Data data;
    for (auto &&primitive : gmesh.primitives) {
      auto &indices = asset.accessors[primitive.indicesAccessor.value()];

      Mesh::Surface surface;
      surface.mStartIndex = data.indices.size();
      surface.mCount = indices.count;

      fastgltf::iterateAccessor<uint32_t>(asset, indices, [&](uint32_t idx) {
        data.indices.push_back(idx + surface.mStartIndex);
      });

      auto &positions =
          asset.accessors[primitive.findAttribute(AttrPosition)->accessorIndex];
      fastgltf::iterateAccessor<glm::vec3>(asset, positions, [&](auto &&pos) {
        data.vertices.push_back({.position = pos});
        // Vulkan's Y coordinate is inverted compared to OpenGL and GLTF
        // FIXME: This fucks up winding order and probably other things
        data.vertices.back().position.y = -data.vertices.back().position.y;
      });

#define UPSERT_ATTR(name, field, type)                                         \
  {                                                                            \
    auto attr = primitive.findAttribute(name);                                 \
    if (attr != primitive.attributes.end()) {                                  \
      auto &access = asset.accessors[attr->accessorIndex];                     \
      fastgltf::iterateAccessorWithIndex<type>(                                \
          asset, access, [&](auto &&value, size_t index) {                     \
            data.vertices[index].field = value;                                \
          });                                                                  \
    }                                                                          \
  }
      UPSERT_ATTR(AttrNormal, normal, glm::vec3)
      UPSERT_ATTR(AttrUv, uv, glm::vec2)
      UPSERT_ATTR(AttrColor, color, glm::vec4)

      // TODO: Remove temp code
      for (auto &vtx : data.vertices) {
        vtx.color = (glm::vec4(vtx.normal, 1.0f) + glm::vec4(1.0f)) / 2.0f;
      }

      out.emplace_back(std::make_shared<Mesh>(gmesh.name, std::move(data)));
    }
  }
  return out;
}

Mesh::Mesh(std::string_view name, Data data)
    : name(name), mData(std::move(data)) {
  auto &handle = VulkanHandle::get();
  const auto indexSize = mData.indices.size() * sizeof(uint32_t);
  const auto vertexSize = mData.vertices.size() * sizeof(interop::Vertex);

  // Add eTransferDst to both buffers so we can upload to them
  mIndexBuffer.allocate(handle.mAllocator, indexSize,
                        vk::BufferUsageFlagBits::eIndexBuffer |
                            vk::BufferUsageFlagBits::eTransferDst,
                        VMA_MEMORY_USAGE_GPU_ONLY);
  mVertexBuffer.allocate(handle.mAllocator, vertexSize,
                         vk::BufferUsageFlagBits::eVertexBuffer |
                             vk::BufferUsageFlagBits::eTransferDst,
                         VMA_MEMORY_USAGE_GPU_ONLY);

  // TODO: Should we reuse the staging buffer?
  auto stagingBuffer =
      Buffer::transferBuffer(handle.mAllocator, vertexSize + indexSize);
  void *gpuData = stagingBuffer.getAllocationInfo().pMappedData;
  assert(gpuData != nullptr);
  memcpy(gpuData, mData.vertices.data(), vertexSize);
  memcpy(reinterpret_cast<uint8_t *>(gpuData) + vertexSize,
         mData.indices.data(), indexSize);

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
  auto &handle = VulkanHandle::get();
  mVertexBuffer.free(handle.mAllocator);
  mIndexBuffer.free(handle.mAllocator);
}

} // namespace selwonk::vulkan
