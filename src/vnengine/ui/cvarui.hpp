#pragma once

#include <imgui.h>

#include <vnassets/image.hpp>
#include <vncore/cvar.hpp>

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
  assets::ImageBase::Handle mAlertHandle;
  ImTextureID mAlertIcon;
};
} // namespace selwonk::ui
