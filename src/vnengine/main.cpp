#include "core/settings.hpp"
#include "core/window.hpp"
#include "vk/vulkanengine.hpp"
#include "vk/vulkanhandle.hpp"
#include <vncore/cvar.hpp>

selwonk::core::Cvar::Int WindowWidth("window.width", 1280, "Window width");
selwonk::core::Cvar::Int WindowHeight("window.height", 720, "Window height");

int main(int argc, char** argv) {
  bool quit = selwonk::core::Cvar::get().parseCli(argc, argv);
  if (quit)
    return 0;
  selwonk::core::Settings settings;

  selwonk::core::Window window(WindowWidth, WindowHeight);
  selwonk::vulkan::VulkanHandle handle(settings, window);
  selwonk::vulkan::VulkanEngine engine(settings, window, handle);
  engine.run();

  return 0;
}
