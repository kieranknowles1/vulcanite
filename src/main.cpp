#include "vk/vulkanengine.hpp"

int main() {
  selwonk::vk::VulkanEngine engine;
  engine.init({});
  engine.run();
  engine.shutdown();

  return 0;
}
