#pragma once

#include <vnvulkan/shader.hpp>
#include <vnassets/mesh.hpp>
#include <vnassets/debug.hpp>
#include "vulkan/vulkan.hpp"
#include <vncore/bumpallocator.hpp>
#include <vncore/singleton.hpp>

namespace selwonk::vulkan {
  // TODO: This shouldn't be singleton once render system owns pipeline init
class VulkanDebugRenderer : public core::Singleton<VulkanDebugRenderer> {
public:
  // TODO: Use the current frame's draw buffer
  const static constexpr size_t MaxDebugLines = 1024 * 1024;
  const static constexpr size_t DebugBufferSize =
      MaxDebugLines * sizeof(interop::Vertex) * 2;

  VulkanDebugRenderer();
  ~VulkanDebugRenderer();

  void draw(vk::CommandBuffer cmd, vk::DescriptorSet drawDescriptors, const assets::Debug& debugData);

  void initPipelines();

private:
  Pipeline mPipeline;
  Pipeline mSolidPipeline;

  // TODO: Does this need to be frame-level data?
  assets::MeshData::VertexHandle mBuffer;
  core::BumpAllocator mAllocator;
};
} // namespace selwonk::vulkan
