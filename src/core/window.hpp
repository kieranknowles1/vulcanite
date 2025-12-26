#pragma once

#include "settings.hpp"
#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>

namespace selwonk::core {
class Window {
public:
  Window(const Settings &settings);
  ~Window();

  void update();
  bool quitRequested() { return mQuitRequested; }

  glm::uvec2 getSize() { return mSize; }
  SDL_Window *getSdl() { return mWindow; }

private:
  SDL_Window *mWindow;
  glm::uvec2 mSize;

  bool mQuitRequested = false;
};
} // namespace selwonk::core
