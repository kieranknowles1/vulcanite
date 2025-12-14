#include "window.hpp"
#include <SDL3/SDL_init.h>

namespace selwonk::vk {
Window::Window(Settings settings) {
  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulkan Engine", settings.size.x, settings.size.y,
                             SDL_WINDOW_VULKAN);
}

Window::~Window() { SDL_DestroyWindow(mWindow); }
} // namespace selwonk::vk
