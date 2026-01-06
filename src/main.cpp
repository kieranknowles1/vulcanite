#include "core/cli.hpp"
#include "core/settings.hpp"
#include "core/window.hpp"
#include "vk/vulkanengine.hpp"
#include "vk/vulkanhandle.hpp"

int main(int argc, char** argv) {
  selwonk::core::Cli cli(argc, argv);
  if (cli.help) {
    cli.parser.printHelp();
    return 0;
  }
  selwonk::core::Settings settings;

  selwonk::core::Window window(settings);
  selwonk::vulkan::VulkanHandle handle(settings, window);
  selwonk::vulkan::VulkanEngine engine(cli, settings, window, handle);
  engine.run();

  return 0;
}
