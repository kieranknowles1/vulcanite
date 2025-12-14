#include "vk/vulkanengine.hpp"
#include "vk/vulkanhandle.hpp"
#include "vk/window.hpp"

int main() {
  selwonk::vk::Window window({});
  selwonk::vk::VulkanHandle vulkan({}, window);
  selwonk::vk::VulkanEngine engine(window, vulkan, {});
  engine.run();
  engine.shutdown();

  return 0;
}
