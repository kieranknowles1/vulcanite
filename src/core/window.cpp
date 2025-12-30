#include "window.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <fmt/base.h>
#include <imgui_impl_sdl3.h>

namespace selwonk::core {
Window::Window(const Settings &settings) : mSize(settings.initialSize) {
  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulcanite", mSize.x, mSize.y,
                             SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_MOUSE_GRABBED);
}

void Window::update() {
  mKeyboard.update();
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
      mKeyboard.receiveEvent(e);
      break;
    }
  }

  mQuitRequested |= mKeyboard.getDigital(Keyboard::DigitalControl::Quit);

  ImGui_ImplSDL3_NewFrame();
}

Window::~Window() {
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}
} // namespace selwonk::core
