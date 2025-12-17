#include "vk/vulkanengine.hpp"

int main() {
  selwonk::vulkan::VulkanEngine engine;
  engine.init({});
  engine.run();
  engine.shutdown();

  return 0;
}
