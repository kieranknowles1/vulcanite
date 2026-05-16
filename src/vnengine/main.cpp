// TODO: WASM
#include <vncore/cvar.hpp>
#include <vnsdl/window.hpp>

selwonk::core::Cvar::Int WindowWidth("window.width", 1280, "Window width");
selwonk::core::Cvar::Int WindowHeight("window.height", 720, "Window height");

#ifndef VN_WASM
#include "vk/vulkanengine.hpp"
#include <vnvulkan/vulkanhandle.hpp>

#else
#include <iostream>
#endif

int main(int argc, char** argv) {
// TODO: WASM
#ifndef VN_WASM
  bool quit = selwonk::core::Cvar::get().parseCli(argc, argv);
  if (quit)
    return 0;
#endif

  selwonk::sdl::Window window(WindowWidth, WindowHeight);

#ifndef VN_WASM
  selwonk::vulkan::VulkanHandle handle(window);
  selwonk::vulkan::VulkanEngine engine(window, handle);
  engine.run();

#endif
  return 0;
}
