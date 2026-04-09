#pragma once

#include "keyboard.hpp"
#include "vncore/cvar.hpp"
#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>

namespace selwonk::core {
class Window {
public:
  Window(Cvar::Int& width, Cvar::Int& height);
  ~Window();

  void update();
  bool quitRequested() { return mQuitRequested; }

  glm::ivec2 getSize() { return {mWidth.value(), mHeight.value()}; }
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
  void updateSize();

  SDL_Window* mWindow;
  Cvar::Int& mWidth;
  Cvar::Int& mHeight;

  bool mQuitRequested = false;
  bool mResized = false;
  bool mMouseVisible = false;

  Keyboard mKeyboard;
};
} // namespace selwonk::core
