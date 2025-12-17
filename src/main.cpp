#include "vk/vulkanengine.hpp"

int main() {
  selwonk::core::Window window;
  selwonk::vk::VulkanEngine engine(window);
  engine.init();
  engine.run();
  engine.shutdown();

  return 0;
}
