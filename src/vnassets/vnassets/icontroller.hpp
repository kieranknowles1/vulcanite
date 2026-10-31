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

  enum class DigitalControl {
    Quit,          // TEMP
    SpawnItem,     // TEMP
    AddCameraNode, // TEMP
    ReloadScene,   // TEMP
    ToggleMouse,
    ToggleConsole,
  };

  virtual ~IController() = default;
  virtual bool getDigital(DigitalControl control) const = 0;
  virtual float getAnalog(AnalogControl control) const = 0;
  virtual void update() = 0;
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    IController::DigitalControl,
    {
        {IController::DigitalControl::Quit, "Quit"},
        {IController::DigitalControl::SpawnItem, "SpawnItem"},
        {IController::DigitalControl::AddCameraNode, "AddCameraNode"},
        {IController::DigitalControl::ReloadScene, "ReloadScene"},
        {IController::DigitalControl::ToggleMouse, "ToggleMouse"},
        {IController::DigitalControl::ToggleConsole, "ToggleConsole"},
    });

// TODO: This would be a great place for codegen/reflection
NLOHMANN_JSON_SERIALIZE_ENUM(
    IController::AnalogControl,
    {
        {IController::AnalogControl::MoveForwardBackward,
         "MoveBackwardForward"},
        {IController::AnalogControl::MoveLeftRight, "MoveLeftRight"},
        {IController::AnalogControl::LookUpDown, "LookUpDown"},
        {IController::AnalogControl::LookLeftRight, "LookLeftRight"},
        {IController::AnalogControl::SpeedChange, "SpeedChange"},
    });
} // namespace selwonk::assets
