#include "mesh.hpp"

#include "buffer.hpp"
#include "vulkan/vulkan.hpp"

namespace selwonk::vulkan {

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
  Buffer stagingBuffer;
  stagingBuffer.allocate(handle.mAllocator, vertexSize + indexSize,
                         vk::BufferUsageFlagBits::eTransferSrc,
                         VMA_MEMORY_USAGE_CPU_ONLY);
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
