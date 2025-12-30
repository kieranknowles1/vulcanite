#pragma once

#include <bitset>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <glm/ext/vector_float2.hpp>

namespace selwonk::core {
class Keyboard {
public:
  enum class AnalogControl {
    MoveForwardBackward,
    MoveLeftRight,

    LookUpDown,
    LookLeftRight,
  };

  enum class DigitalControl {
    Quit,
    SpawnItem,
  };

  bool getDigital(DigitalControl control) const;
  float getAnalog(AnalogControl control) const;

  void receiveEvent(SDL_Event event);
  // Prepare for next frame, before receiving events.
  void update();

private:
  template <typename T> struct HistoryBuffer {
    T current;
    T previous;

    void nextFrame() { previous = current; }
  };

  // Returns true if the key is currently pressed down.
  bool keyDown(SDL_Scancode key) const { return mKeys.current.test(key); }
  // Returns true if the key was just pressed down this frame.
  bool keyTapped(SDL_Scancode key) const {
    return keyDown(key) && !mKeys.previous.test(key);
  }

  bool mouseButtonDown(SDL_MouseButtonFlags button) const {
    return mMouseButtons.current & button;
  }
  bool mouseButtonTapped(SDL_MouseButtonFlags button) const {
    return mouseButtonDown(button) && !(mMouseButtons.previous & button);
  }

  HistoryBuffer<std::bitset<SDL_SCANCODE_COUNT>> mKeys;
  HistoryBuffer<SDL_MouseButtonFlags> mMouseButtons;
  glm::vec2 mMouseDelta;
};
} // namespace selwonk::core
