#include "window.hpp"

#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>

namespace selwonk::core {
Window::Window(const Settings &settings) : mSize(settings.initialSize) {
  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulcanite", mSize.x, mSize.y,
                             SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
}

void Window::update() {
  mResized = false;
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    ImGui_ImplSDL3_ProcessEvent(&e);

    switch (e.type) {
    case SDL_EVENT_QUIT:
      mQuitRequested = true;
      break;
    case SDL_EVENT_WINDOW_RESIZED:
      mSize = {e.window.data1, e.window.data2};
      mResized = true;
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
