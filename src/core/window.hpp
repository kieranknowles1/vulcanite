#pragma once

#include <SDL3/SDL_video.h>
#include <glm/vec2.hpp>

namespace selwonk::core {
class Window {
public:
  Window();
  ~Window();

  glm::uvec2 getCurrentSize() const { return mCurrentSize; }
  SDL_Window *getSdl() { return mWindow; }

private:
  SDL_Window *mWindow;

  glm::uvec2 mCurrentSize;
};
} // namespace selwonk::core
