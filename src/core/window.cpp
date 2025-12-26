#include "window.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>

namespace selwonk::core {
Window::Window(const Settings &settings) : mSize(settings.initialSize) {
  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulkanite", mSize.x, mSize.y, SDL_WINDOW_VULKAN);
}

void Window::update() {
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    ImGui_ImplSDL3_ProcessEvent(&e);

    switch (e.type) {
    case SDL_EVENT_QUIT:
      mQuitRequested = true;
      break;
    default:
      break;
    }
  }
  ImGui_ImplSDL3_NewFrame();
}

Window::~Window() {
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}
} // namespace selwonk::core
