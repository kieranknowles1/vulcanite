#include "usageui.hpp"

#include <imgui.h>

#include <vncore/util.hpp>
#include <vncore/platform.hpp>

#if VN_RENDERER == VN_RENDER_VULKAN
#include <vnvulkan/vulkanrenderpipeline.hpp>
#endif

namespace selwonk::ui {
void UsageUi::drawImpl()
{
  ImGui::LabelText("Textures", "%zu/%i",
    mHandles.textureSize(), mHandles.textureCapacity());
  ImGui::LabelText("Samplers", "%i/%i",
    mHandles.samplerSize(), mHandles.samplerCapacity());
  ImGui::LabelText("Vertex Buffers", "%i/%i",
    mHandles.vertexBufferSize(), mHandles.vertexBufferCapacity());
  ImGui::LabelText("Index Buffers", "%i/%i",
    mHandles.indexBufferSize(), mHandles.indexBufferCapacity());
  ImGui::LabelText("Materials", "%i/%i",
    mHandles.materialSize(), mHandles.materialCapacity());

  // TODO: This is vulkan specific
#if VN_RENDERER == VN_RENDER_VULKAN
  auto& frameData = vulkan::VulkanRenderPipeline::get().getCurrentFrame();
  ImGui::LabelText(
    "Frame Data", "%s/%s",
    core::util::formatFilesize(frameData.mFrameData.offset()).c_str(),
    core::util::formatFilesize(frameData.mFrameData.capacity()).c_str());
#endif

  size_t ram = core::Platform::getMemoryUsage();
  ImGui::LabelText("Memory", "%s", core::util::formatFilesize(ram).c_str());

#ifdef VN_LOGCOMPONENTSTATS
  std::apply(
    [](const auto&... componentArrays) {
      ((ImGui::LabelText(
        componentArrays.getTypeName(), "Count: %zd, Capacity: %zd",
        componentArrays.size(), componentArrays.capacity())),
        ...);
    },
    mEcs.getComponentArrays());
#endif
}

}

