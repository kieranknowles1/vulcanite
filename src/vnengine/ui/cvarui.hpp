#pragma once

#include <vnassets/image.hpp>
#include <vncore/cvar.hpp>

#include "element.hpp";

namespace selwonk::ui {
class CvarUi final : public Element {
public:
  CvarUi(core::Cvar& vars);
  ~CvarUi() override;

  const char* name() const { return "CVars"; }
  void drawImpl() override;

private:
  void displayInputBox(core::Cvar::VarBase* var);

  void displayEditor(core::Cvar::VarBase* var);

  core::Cvar& mVars;

  assets::ImageBase::GuiHandle mAlertIcon;
};
} // namespace selwonk::ui
