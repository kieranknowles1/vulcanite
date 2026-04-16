#pragma once

#include "../vk/texturemanager.hpp"
#include "vncore/cvar.hpp"
#include <imgui.h>

namespace selwonk::core {
class CvarUi {
public:
  CvarUi(Cvar& vars);
  ~CvarUi();

  void displayUi();

private:
  void displayInputBox(Cvar::VarBase* var);

  void displayEditor(Cvar::VarBase* var);

  Cvar& mVars;

  // TODO: Move UI out of core and into engine
  vulkan::TextureManager::Handle mAlertHandle;
  ImTextureID mAlertIcon;
};
} // namespace selwonk::core
