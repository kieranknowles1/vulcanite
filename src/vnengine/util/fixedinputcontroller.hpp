#pragma once

#include "vncore/vfs.hpp"
#include <vnassets/icontroller.hpp>

#include <set>

namespace selwonk::engine::util {
// Specialised controller that executes a fixed sequence of inputs defined by a
// file. Think Source engine demos. Useful for integration tests.
class FixedInputController final : public assets::IController {
public:
  struct InputSequence {
    struct Keyframe {
      std::set<DigitalControl> mActiveDigital;
      int mDurationFrames;
      std::map<AnalogControl, float> mActiveAnalog;

      NLOHMANN_DEFINE_TYPE_INTRUSIVE(Keyframe, mActiveDigital, mActiveAnalog,
                                     mDurationFrames);
    };
    // TODO: Enforce timestep matches cvar
    float mTimestep;
    std::vector<Keyframe> mKeyframes;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InputSequence, mKeyframes, mTimestep);
  };

  FixedInputController(core::Vfs& vfs, core::Vfs::Path source);

  bool getDigital(DigitalControl control) const override;
  float getAnalog(AnalogControl control) const override;
  void update() override;

  InputSequence mSequence;
  int mFrameIndex = 0;
  int mFrameElapsed = 0;
};
} // namespace selwonk::engine::util
