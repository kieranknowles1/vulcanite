#pragma once

#include "../vk/texturemanager.hpp"
#include "vncore/cvar.hpp"
#include <imgui.h>

namespace selwonk::ui {
class CvarUi {
public:
  CvarUi(core::Cvar& vars);
  ~CvarUi();

  void displayUi();

private:
  void displayInputBox(core::Cvar::VarBase* var);

  void displayEditor(core::Cvar::VarBase* var);

  core::Cvar& mVars;

  // TODO: Move UI out of core and into engine
  vulkan::TextureManager::Handle mAlertHandle;
  ImTextureID mAlertIcon;
};
} // namespace selwonk::ui
