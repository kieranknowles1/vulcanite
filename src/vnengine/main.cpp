// TODO: WASM
#include <vncore/cvar.hpp>
#include <vnsdl/window.hpp>
#include <spdlog/spdlog.h>

#ifndef VN_WASM
#include "vk/vulkanengine.hpp"
#include "vk/vulkanhandle.hpp"
#endif

selwonk::core::Cvar::Int WindowWidth("window.width", 1280, "Window width");
selwonk::core::Cvar::Int WindowHeight("window.height", 720, "Window height");

selwonk::core::Cvar::Enum<spdlog::level::level_enum> LogLevel(
  "log.level", spdlog::level::info,
  "Minimum level of messages to log",
  {
    {"trace", "", spdlog::level::trace},
    {"debug", "", spdlog::level::debug},
    {"info", "", spdlog::level::info},
    {"warn", "", spdlog::level::warn},
    {"err", "", spdlog::level::err},
    {"critical", "", spdlog::level::critical},
    {"off", "", spdlog::level::off},
  });

static void initLogging() {
  spdlog::set_level(LogLevel.value());
  // TODO: Provide the correct type in EnumVar change callback. Probably need to rethink how it is implemented
  LogLevel.addChangeCallback([](auto level) {
    spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
  });
}

int main(int argc, char** argv) {
  initLogging();

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
