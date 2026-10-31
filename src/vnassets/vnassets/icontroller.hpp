#pragma once

#include <nlohmann/json.hpp>

namespace selwonk::assets {
class IController {
public:
  enum class AnalogControl {
    MoveForwardBackward,
    MoveLeftRight,

    LookUpDown,
    LookLeftRight,

    SpeedChange, // TEMP
  };
  // TODO: This would be a great place for codegen/reflection
  NLOHMANN_JSON_SERIALIZE_ENUM(
      AnalogControl,
      {
          {AnalogControl::MoveForwardBackward, "MoveBackwardForward"},
          {AnalogControl::MoveLeftRight, "MoveLeftRight"},
          {AnalogControl::LookUpDown, "LookUpDown"},
          {AnalogControl::LookLeftRight, "LookLeftRight"},
          {AnalogControl::SpeedChange, "SpeedChange"},
      });

  enum class DigitalControl {
    Quit,          // TEMP
    SpawnItem,     // TEMP
    AddCameraNode, // TEMP
    ReloadScene,   // TEMP
    ToggleMouse,
    ToggleConsole,
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(
      DigitalControl, {
                          {DigitalControl::Quit, "Quit"},
                          {DigitalControl::SpawnItem, "SpawnItem"},
                          {DigitalControl::AddCameraNode, "AddCameraNode"},
                          {DigitalControl::ReloadScene, "ReloadScene"},
                          {DigitalControl::ToggleMouse, "ToggleMouse"},
                          {DigitalControl::ToggleConsole, "ToggleConsole"},
                      });

  virtual ~IController() = default;
  virtual bool getDigital(DigitalControl control) const = 0;
  virtual float getAnalog(AnalogControl control) const = 0;
  virtual void update() = 0;
};
} // namespace selwonk::assets
