#include "window.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_video.h>
#include <fmt/base.h>

// TODO: WASM
#ifndef VN_WASM
#include <imgui_impl_sdl3.h>
#endif

namespace selwonk::core {
Window::Window(Cvar::Int& width, Cvar::Int& height)
    : mWidth(width), mHeight(height) {
  SDL_Init(SDL_INIT_VIDEO);
  mWindow = SDL_CreateWindow("Vulcanite", width.value(), height.value(),
                             SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_MOUSE_GRABBED);
  SDL_SetWindowRelativeMouseMode(mWindow, true);

  auto validate = [](int size) -> std::optional<std::string> {
    if (size <= 0)
      return "Size must be positive";
    return std::nullopt;
  };
  auto update = [this](int _size) { updateSize(); };
  width.addChangeCallback(update);
  width.addValidationCallback(validate);
  height.addChangeCallback(update);
  height.addValidationCallback(validate);
}

void Window::update() {
  mKeyboard.update();
  mResized = false;
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
// TODO: WASM
#ifndef VN_WASM
    ImGui_ImplSDL3_ProcessEvent(&e);
#endif

    switch (e.type) {
    case SDL_EVENT_QUIT:
      mQuitRequested = true;
      break;
    case SDL_EVENT_WINDOW_RESIZED:
      mResized = true;
      mWidth.setValue(e.window.data1);
      mHeight.setValue(e.window.data2);
      break;
    default:
      mKeyboard.receiveEvent(e);
      break;
    }
  }

  mQuitRequested |= mKeyboard.getDigital(Keyboard::DigitalControl::Quit);
// TODO: WASM
#ifndef VN_WASM
  ImGui_ImplSDL3_NewFrame();
#endif
}

void Window::updateSize() {
  SDL_SetWindowSize(mWindow, mWidth.value(), mHeight.value());
}

Window::~Window() {
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}
} // namespace selwonk::core
