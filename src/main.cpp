#include "vk/vulkanengine.hpp"
#include "vk/window.hpp"

int main() {
  selwonk::vk::Window window({});
  selwonk::vk::VulkanEngine engine(window);
  engine.init({});
  engine.run();
  engine.shutdown();

  return 0;
}
