#pragma once

#include <array>

#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>

#include "../vfs.hpp"
#include "imguiwrapper.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vulkanhandle.hpp"

#include "../../assets/shaders/interop.h"

namespace selwonk::vulkan {
class VulkanEngine {
public:
  struct EngineSettings {
    glm::uvec2 size = glm::ivec2(1280, 720);
    VulkanHandle::Settings mVulkan;
  };

  struct FrameData {
    vk::CommandPool mCommandPool;     // Allocator for command buffers
    vk::CommandBuffer mCommandBuffer; // Pool of commands yet to be submitted

    vk::Semaphore
        mSwapchainSemaphore; // Tell the GPU when the GPU is done rendering
    vk::Fence mRenderFence;  // Tell the CPU when the GPU is done rendering

    void init(VulkanHandle &handle);
    void destroy(VulkanHandle &handle);
  };

  static constexpr unsigned int BufferCount = 2;

  static VulkanEngine &get();

  VulkanEngine();
  ~VulkanEngine();

  void init(EngineSettings settings);
  void run();
  void shutdown();

  FrameData &getCurrentFrame() {
    return mFrameData[mFrameNumber % BufferCount];
  }

  VulkanHandle &getVulkan() { return mHandle; }
  Vfs &getVfs() { return *mVfs; }

private:
  void initCommands();
  void initDescriptors();

  void draw();
  void drawBackground(vk::CommandBuffer cmd);
  void drawScene(vk::CommandBuffer cmd);

  EngineSettings mSettings;

  SDL_Window *mWindow;
  VulkanHandle mHandle;

  Image mDrawImage;
  vk::Extent2D mDrawExtent;

  DescriptorAllocator mGlobalDescriptorAllocator;
  vk::DescriptorSet mDrawImageDescriptors;
  vk::DescriptorSetLayout mDrawImageDescriptorLayout;

  ImguiWrapper mImgui;

  ComputePipeline mGradientShader;
  interop::GradientPushConstants mPushConstants = {
      .leftColor = {0.0f, 0.0f, 1.0f, 1.0f},
      .rightColor = {1.0f, 0.0f, 0.0f, 1.0f},
  };
  interop::VertexPushConstants mVertexPushConstants;

  Pipeline mTrianglePipeline;

  std::array<FrameData, BufferCount> mFrameData;

  std::unique_ptr<Vfs> mVfs;
  std::vector<Mesh> mFileMeshes;

  unsigned int mFrameNumber = 0;
};
} // namespace selwonk::vulkan
