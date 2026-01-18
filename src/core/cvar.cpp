#include "cvar.hpp"
#include <imgui.h>

namespace selwonk::core {

template <> void Cvar::Var<int>::displayEdit() {
  ImGui::InputInt(mName.c_str(), &mPendingChange);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("%s", mDescription.c_str());
  }
}

void Cvar::displayUi() {
  if (ImGui::Begin("CVar")) {
    for (auto& var : mVars) {
      var.second->displayEdit();
    }

    bool anyDirty = false;
    for (auto& var : mVars) {
      if (var.second->dirty()) {
        anyDirty = true;
        break;
      }
    }

    if (ImGui::Button(anyDirty ? "Apply" : "No Changes")) {
      for (auto& var : mVars) {
        var.second->apply();
      }
    }
  }
  ImGui::End();
}

} // namespace selwonk::core
