#pragma once

#include <SDL3/SDL_video.h>
#include <glm/ext/vector_uint2.hpp>

namespace selwonk::vk {
class Window {
public:
  struct Settings {
    glm::uvec2 size = glm::uvec2(1280, 720);
  };

  Window(Settings settings);
  ~Window();

  SDL_Window *getSdl() { return mWindow; }

private:
  SDL_Window *mWindow;
};
} // namespace selwonk::vk
