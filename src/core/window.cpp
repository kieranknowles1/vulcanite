#include "window.hpp"
#include <SDL3/SDL_init.h>

#include "settings.hpp"

namespace selwonk::core {
Window::Window() {
  auto &settings = Settings::get();
  mCurrentSize = settings.windowSize;
  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulkanite", mCurrentSize.x, mCurrentSize.y,
                             SDL_WINDOW_VULKAN);
}

Window::~Window() {
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}

} // namespace selwonk::core
