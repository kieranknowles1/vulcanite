#include "core/settings.hpp"
#include "core/window.hpp"
#include "vk/vulkanengine.hpp"

int main() {
  selwonk::core::Settings settings;

  selwonk::core::Window window(settings);
  selwonk::vulkan::VulkanEngine engine(window);
  engine.init(settings);
  engine.run();
  engine.shutdown();

  return 0;
}
