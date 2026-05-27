#include "window.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_video.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

// TODO: WASM
#ifndef VN_WASM
#include <backends/imgui_impl_sdl3.h>
#endif

namespace selwonk::sdl {
Window::Window(core::Cvar::Int& width, core::Cvar::Int& height)
    : mWidth(width), mHeight(height) {
#ifdef VN_WASM
  auto platformFlag = 0;
  // Attach window to canvas with ID `vulcanite`
  SDL_SetHint(SDL_HINT_EMSCRIPTEN_CANVAS_SELECTOR, "#vulcanite");
#else
  auto platformFlag = SDL_WINDOW_VULKAN;
#endif

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    spdlog::critical("{}", SDL_GetError());
    throw std::runtime_error("Failed to init SDL");
  }
  mWindow = SDL_CreateWindow("Vulcanite", width.value(), height.value(),
                             platformFlag | SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_MOUSE_GRABBED);
  if (mWindow == nullptr) {
    spdlog::critical("{}", SDL_GetError());
    throw std::runtime_error("Failed to create window");
  }
  SDL_SetWindowRelativeMouseMode(mWindow, true);

  auto validate = [](int size) -> std::optional<std::string> {
    if (size <= 0)
      return "Size must be positive";
    return std::nullopt;
  };
  auto update = [this](int _size) { updateSize(); };
  width.getStore().addChange(update);
  width.getStore().addValidate(validate);
  height.getStore().addChange(update);
  height.getStore().addValidate(validate);
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
} // namespace selwonk::sdl
