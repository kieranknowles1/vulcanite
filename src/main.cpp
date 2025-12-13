#include "vk/vulkanengine.hpp"

int main() {
  selwonk::vul::VulkanEngine engine;
  engine.init({});
  engine.run();
  engine.shutdown();

  return 0;
}
