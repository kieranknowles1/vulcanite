#include "cvar.hpp"
#include "vulkan/vulkan.hpp"

#include <imgui.h>

namespace selwonk::core {

// TODO: Move GUI out of core
template <> void Cvar::Int::displayInputBox() {
  ImGui::InputInt(mName.c_str(), &mPendingChange);
}

template <> void Cvar::Float::displayInputBox() {
  ImGui::InputFloat(mName.c_str(), &mPendingChange);
}

template <> void Cvar::Bool::displayInputBox() {
  ImGui::Checkbox(mName.c_str(), &mPendingChange);
}

// TODO: Temp
template <> void Cvar::Enum<vk::PresentModeKHR>::displayInputBox() {
  const char* selected;
  for (auto& opt : mOptions) {
    if (static_cast<Backing>(opt.value) == mPendingChange) {
      selected = opt.name.c_str();
    }
  }

  if (ImGui::BeginCombo(mName.c_str(), selected)) {
    for (auto& opt : mOptions) {
      bool selected = ImGui::Selectable(
          opt.name.c_str(),
          opt.value == static_cast<vk::PresentModeKHR>(mPendingChange));
      if (selected) {
        mPendingChange = static_cast<Backing>(opt.value);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", opt.description.c_str());
      }
    }
    ImGui::EndCombo();
  }
}

template <typename T> void Cvar::Var<T>::displayEdit() {
  // A label's name is its ID, suffixing with ##mName ensures uniqueness
  // without affecting display
  std::string label = "Reset##" + mName;
  if (ImGui::Button(label.c_str())) {
    mPendingChange = mDefault;
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(128);
  displayInputBox();
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("%s", mDescription.c_str());
  }

  if (hasFlag(Flags::InitOnly)) {
    // TODO: Use an icon for this
    ImGui::SameLine();
    ImGui::Text("Init Only");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Setting requires a restart to apply.");
    }
  }

  auto valid = validate(mPendingChange);
  if (valid != std::nullopt) {
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", valid->c_str());
  }
}

bool Cvar::parseCli(int argc, char** argv) {
  if (argc <= 1)
    return false;                  // No args
  std::string_view arg1 = argv[1]; // argv[0] is the process name
  if (arg1 == "-h" || arg1 == "--help" || arg1 == "help") {
    fmt::println("Usage: {} [name value]... -- set CVars on startup", argv[0]);
    fmt::println("known CVars:");
    for (auto& var : mVars) {
      // TODO: Display/parse string values for enum options
      fmt::println("  {} = {}: {}", var.second->getName(),
                   var.second->toString(), var.second->getDescription());
    }
    return true;
  }

  if (argc % 2 != 1) {
    fmt::println("Expected arguments to follow [name value]");
    return true;
  }

  bool bad = false;
  int count = (argc - 1) / 2;
  for (int i = 0; i < count; i++) {
    auto name = argv[i * 2 + 1];
    auto value = argv[i * 2 + 2];

    auto var = mVars.find(name);
    if (var == mVars.end()) {
      fmt::println("Unknown CVar '{}'", name);
      bad = true;
    }

    bool ok = var->second->setString(value);
    if (!ok) {
      fmt::println("Invalid value '{}' for '{}'", value, name);
      bad = true;
    }
  }
  return bad;
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
