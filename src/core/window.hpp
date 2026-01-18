#pragma once

#include "keyboard.hpp"
#include "settings.hpp"
#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>

namespace selwonk::core {
class Window {
public:
  Window(const Settings& settings);
  ~Window();

  void update();
  bool quitRequested() { return mQuitRequested; }

  glm::uvec2 getSize() { return mSize; }
  bool resized() { return mResized; }
  SDL_Window* getSdl() { return mWindow; }

  bool mouseVisible() const { return mMouseVisible; }
  void setMouseVisible(bool state) {
    SDL_SetWindowRelativeMouseMode(mWindow, !state);
    SDL_SetWindowMouseGrab(mWindow, !state);
    mMouseVisible = state;
  }

  const Keyboard& getKeyboard() const { return mKeyboard; }

private:
  SDL_Window* mWindow;
  glm::uvec2 mSize;

  bool mQuitRequested = false;
  bool mResized = false;
  bool mMouseVisible = false;

  Keyboard mKeyboard;
};
} // namespace selwonk::core
