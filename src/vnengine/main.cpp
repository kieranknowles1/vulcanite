// TODO: WASM
#include <spdlog/spdlog.h>
#include <vncore/cvar.hpp>
#include <vnsdl/window.hpp>

#ifndef VN_WASM
#include "vk/vulkanengine.hpp"
#include "vk/vulkanhandle.hpp"
#endif

selwonk::core::Cvar::Int WindowWidth("window.width", 1280, "Window width",
                                     selwonk::core::Cvar::Flags::Unsigned);
selwonk::core::Cvar::Int WindowHeight("window.height", 720, "Window height",
                                      selwonk::core::Cvar::Flags::Unsigned);

selwonk::core::Cvar::Enum<spdlog::level::level_enum>
    LogLevel("log.level", spdlog::level::info,
             "Minimum level of messages to log",
             {
                 // TODO: Compile with min level trace and omit lower than
                 // supported levels
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
  LogLevel.getStore().addChange([](auto level) { spdlog::set_level(level); });
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
