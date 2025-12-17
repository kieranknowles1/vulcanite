#include "vk/vulkanengine.hpp"

int main() {
  selwonk::core::Window window;
  selwonk::vk::VulkanHandle handle(window);
  selwonk::vk::VulkanEngine engine(window, handle);
  engine.run();

  return 0;
}
