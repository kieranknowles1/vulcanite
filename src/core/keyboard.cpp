#include "keyboard.hpp"

namespace selwonk::core {

void Keyboard::update() {
  mKeys.nextFrame();
  mMouseButtons.nextFrame();
  mMouseDelta = {0, 0};
}

void Keyboard::receiveEvent(SDL_Event event) {
  switch (event.type) {
  case SDL_EVENT_KEY_DOWN:
    mKeys.current[event.key.scancode] = true;
    break;
  case SDL_EVENT_KEY_UP:
    mKeys.current[event.key.scancode] = false;
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    mMouseButtons.current |= event.button.button;
    break;
  case SDL_EVENT_MOUSE_BUTTON_UP:
    mMouseButtons.current &= ~event.button.button;
    break;
  // FIXME: This is completely wrong, it locks at the screen edge and jerks
  // on the first movement
  case SDL_EVENT_MOUSE_MOTION:
    mMouseDelta = {event.motion.xrel, event.motion.yrel};
    break;
  }
}

bool Keyboard::getDigital(Keyboard::DigitalControl control) const {
  using enum Keyboard::DigitalControl;
  switch (control) {
  case Quit:
    return keyTapped(SDL_SCANCODE_ESCAPE);
  case SpawnItem:
    return keyTapped(SDL_SCANCODE_SPACE);
  }
  assert(false);
}

float Keyboard::getAnalog(Keyboard::AnalogControl control) const {
  using enum Keyboard::AnalogControl;
  switch (control) {
  case MoveForwardBackward:
    return keyDown(SDL_SCANCODE_W) - keyDown(SDL_SCANCODE_S);
  case MoveLeftRight:
    return keyDown(SDL_SCANCODE_A) - keyDown(SDL_SCANCODE_D);
  case LookUpDown:
    return mMouseDelta.y;
  case LookLeftRight:
    return mMouseDelta.x;
  }
  assert(false);
}

} // namespace selwonk::core
