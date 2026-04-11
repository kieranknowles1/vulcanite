// TODO: WASM
#ifndef VN_WASM
#include "core/window.hpp"
#include "vk/vulkanengine.hpp"
#include "vk/vulkanhandle.hpp"
#include <vncore/cvar.hpp>

selwonk::core::Cvar::Int WindowWidth("window.width", 1280, "Window width");
selwonk::core::Cvar::Int WindowHeight("window.height", 720, "Window height");
#else
#include <iostream>
#endif

int main(int argc, char** argv) {
// TODO: WASM
#ifndef VN_WASM
  bool quit = selwonk::core::Cvar::get().parseCli(argc, argv);
  if (quit)
    return 0;

  selwonk::core::Window window(WindowWidth, WindowHeight);
  selwonk::vulkan::VulkanHandle handle(window);
  selwonk::vulkan::VulkanEngine engine(window, handle);
  engine.run();

#else
  std::cout << "Hello from WASM" << std::endl;
#endif
  return 0;
}
