#include "fixedinputcontroller.hpp"
#include "spdlog/spdlog.h"

namespace selwonk::engine::util {
FixedInputController::FixedInputController(core::Vfs& vfs,
                                           core::Vfs::Path source) {
  auto file = vfs.get(source)->open();
  mSequence = nlohmann::json::parse(file);
}

bool FixedInputController::getDigital(DigitalControl control) const {
  if (mFrameIndex >= mSequence.mKeyframes.size()) {
    return control == DigitalControl::Quit;
  }

  return mSequence.mKeyframes[mFrameIndex].mActiveDigital.count(control);
}

float FixedInputController::getAnalog(AnalogControl control) const {
  if (mFrameIndex >= mSequence.mKeyframes.size()) {
    return false;
  }

  auto& active = mSequence.mKeyframes[mFrameIndex].mActiveAnalog;
  auto it = active.find(control);
  return it == active.end() ? 0 : it->second;
}

void FixedInputController::update() {
  if (mFrameIndex >= mSequence.mKeyframes.size()) {
    return;
  }
  mFrameElapsed++;
  if (mFrameElapsed >= mSequence.mKeyframes[mFrameIndex].mDurationFrames) {
    mFrameIndex++;
    mFrameElapsed = 0;
  }
}

} // namespace selwonk::engine::util
