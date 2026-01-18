#include "cvar.hpp"
#include <imgui.h>

namespace selwonk::core {

template <> void Cvar::Var<int>::displayEdit() {
  // A label's name is its ID, suffixing with ##mName ensures uniqueness
  // without affecting display
  std::string label = "Reset##" + mName;
  if (ImGui::Button(label.c_str())) {
    mPendingChange = mDefault;
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(128);
  ImGui::InputInt(mName.c_str(), &mPendingChange);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("%s", mDescription.c_str());
  }

  auto valid = validate(mPendingChange);
  if (valid != std::nullopt) {
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", valid->c_str());
  }
}

void Cvar::displayUi() {
  if (ImGui::Begin("CVar")) {
    for (auto& var : mVars) {
      var.second->displayEdit();
    }

    bool anyDirty = false;
    bool anyBad = false;
    for (auto& var : mVars) {
      if (var.second->dirty()) {
        anyDirty = true;
      }
      if (!var.second->isPendingValid()) {
        anyBad = true;
      }
    }

    if (ImGui::Button(anyDirty ? "Apply" : "No Changes")) {
      for (auto& var : mVars) {
        if (var.second->dirty() && var.second->isPendingValid()) {
          var.second->apply();
        }
      }
    }
    if (anyBad && ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Invalid values will be skipped");
    }
  }
  ImGui::End();
}

} // namespace selwonk::core
