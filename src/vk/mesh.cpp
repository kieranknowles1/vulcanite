#include "mesh.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "buffer.hpp"
#include "fastgltf/tools.hpp"
#include "vulkanengine.hpp"

namespace selwonk::vulkan {

std::vector<Mesh> Mesh::load(VulkanHandle &handle, Vfs::SubdirPath path) {
  auto &vfs = VulkanEngine::get().getVfs();

  std::vector<std::byte> buffer;
  vfs.readfull(Vfs::Meshes / path, buffer);

  auto data = fastgltf::GltfDataBuffer::FromBytes(buffer.data(), buffer.size());
  if (data.error() != fastgltf::Error::None) {
    throw LoadException(data.error());
  }

  auto options = fastgltf::Options::LoadExternalBuffers;

  fastgltf::Parser parser;
  auto load = parser.loadGltf(data.get(), "/");
  if (load.error() != fastgltf::Error::None) {
    throw LoadException(load.error());
  }

  auto asset = std::move(load.get());

  std::vector<Mesh> out;
  for (auto &gmesh : asset.meshes) {
    Mesh mesh;
    mesh.mName = gmesh.name;
    for (auto &&primitive : gmesh.primitives) {
      auto &indices = asset.accessors[primitive.indicesAccessor.value()];

      Mesh::Surface surface;
      surface.mStartIndex = mesh.mIndices.size();
      surface.mCount = indices.count;

      fastgltf::iterateAccessor<uint32_t>(asset, indices, [&](uint32_t idx) {
        mesh.mIndices.push_back(idx + surface.mStartIndex);
      });

      auto &positions =
          asset.accessors[primitive.findAttribute(AttrPosition)->accessorIndex];
      fastgltf::iterateAccessor<glm::vec3>(asset, positions, [&](auto &&pos) {
        mesh.mVertices.push_back({.position = pos});
        // Vulkan's Y coordinate is inverted compared to OpenGL and GLTF
        mesh.mVertices.back().position.y = -mesh.mVertices.back().position.y;
      });

#define UPSERT_ATTR(name, field, type)                                         \
  {                                                                            \
    auto attr = primitive.findAttribute(name);                                 \
    if (attr != primitive.attributes.end()) {                                  \
      auto &access = asset.accessors[attr->accessorIndex];                     \
      fastgltf::iterateAccessorWithIndex<type>(                                \
          asset, access, [&](auto &&value, size_t index) {                     \
            mesh.mVertices[index].field = value;                               \
          });                                                                  \
    }                                                                          \
  }
      UPSERT_ATTR(AttrNormal, normal, glm::vec3)
      UPSERT_ATTR(AttrUv, uv, glm::vec2)
      UPSERT_ATTR(AttrColor, color, glm::vec4)

      // TODO: Remove temp code
      for (auto &vtx : mesh.mVertices) {
        vtx.color = (glm::vec4(vtx.normal, 1.0f) + glm::vec4(1.0f)) / 2.0f;
      }

      mesh.upload(handle);
      out.push_back(mesh);
    }
  }
  return out;
}

void Mesh::upload(VulkanHandle &handle) {
  const auto indexSize = mIndices.size() * sizeof(uint32_t);
  const auto vertexSize = mVertices.size() * sizeof(interop::Vertex);

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
  void *data = stagingBuffer.getAllocationInfo().pMappedData;
  assert(data != nullptr);
  memcpy(data, mVertices.data(), vertexSize);
  memcpy(reinterpret_cast<uint8_t *>(data) + vertexSize, mIndices.data(),
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

void Mesh::free(VulkanHandle &handle) {
  mVertexBuffer.free(handle.mAllocator);
  mIndexBuffer.free(handle.mAllocator);
}

} // namespace selwonk::vulkan
