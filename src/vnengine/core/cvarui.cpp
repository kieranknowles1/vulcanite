#include "cvarui.hpp"
#include "vncore/cvar.hpp"
#include "vulkan/vulkan.hpp"

#include "../vk/image.hpp"
#include "../vk/vulkanengine.hpp"
#include <backends/imgui_impl_vulkan.h>

namespace selwonk::core {

CvarUi::CvarUi(Cvar& vars) : mVars(vars) {
  auto& engine = vulkan::VulkanEngine::get();

  auto alert =
      vulkan::Image::load(engine.getVfs().get("textures/icons/alert.png"));
  mAlertHandle = engine.getTextureManager().insert(alert);

  // TODO: Ref counted wrapper for ImTextureID
  auto id = ImGui_ImplVulkan_AddTexture(
      engine.getTextureManager().getTexture(mAlertHandle).getView(),
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  mAlertIcon = (ImTextureID)id;
}

CvarUi::~CvarUi() {
  // TODO: This segfaults
  // Leaking isn't too much of an issue since we're shutting down anyway
  // ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)mAlertIcon);
  vulkan::VulkanEngine::get().getTextureManager().decRef(mAlertHandle);
}

void CvarUi::displayUi() {
  if (ImGui::Begin("CVar")) {
    for (auto& var : mVars.getVars()) {
      displayInputBox(var.second);
    }

    bool anyDirty = false;
    bool anyBad = false;
    for (auto& var : mVars.getVars()) {
      if (var.second->dirty()) {
        anyDirty = true;
      }
      if (var.second->validatePending() != std::nullopt) {
        anyBad = true;
      }
    }

    if (ImGui::Button(anyDirty ? "Apply" : "No Changes")) {
      for (auto& var : mVars.getVars()) {
        if (var.second->dirty() && var.second->validatePending() == std::nullopt) {
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

void CvarUi::displayEditor(Cvar::VarBase* var) {
  switch (var->getType()) {
  case Cvar::TypeEnum::Int: {
    Cvar::Int* v = (Cvar::Int*)var;
    ImGui::InputInt(v->getName().c_str(), v->getPendingValue());
    break;
  }
  case Cvar::TypeEnum::Float: {
    Cvar::Float* v = (Cvar::Float*)var;
    ImGui::InputFloat(v->getName().c_str(), v->getPendingValue());
    break;
  }
  case Cvar::TypeEnum::Bool: {
    Cvar::Bool* v = (Cvar::Bool*)var;
    ImGui::Checkbox(v->getName().c_str(), v->getPendingValue());
    break;
  }
  case Cvar::TypeEnum::Enum: {
    // TODO: This is 100% undefined behaviour for anything other than present
    // mode :)
    using Var = Cvar::Enum<vk::PresentModeKHR>;
    Var* v = (Var*)var;
    const char* selected = nullptr;
    for (auto& opt : v->getOptions()) {
      if (static_cast<Var::Backing>(opt.value) == *v->getPendingValue()) {
        selected = opt.name.c_str();
      }
    }

    if (ImGui::BeginCombo(v->getName().c_str(), selected)) {
      for (auto& opt : v->getOptions()) {
        bool selected = ImGui::Selectable(
            opt.name.c_str(), opt.value == static_cast<vk::PresentModeKHR>(
                                               *v->getPendingValue()));
        if (selected) {
          v->setPendingValue(static_cast<Var::Backing>(opt.value));
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("%s", opt.description.c_str());
        }
      }
      ImGui::EndCombo();
    }
  }
  }
}

void CvarUi::displayInputBox(Cvar::VarBase* var) {
  // A label's name is its ID, suffixing with ##mName ensures uniqueness
  // without affecting display
  std::string label = "Reset##" + var->getName();
  if (ImGui::Button(label.c_str())) {
    var->setResetPending();
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(128);
  displayEditor(var);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("%s", var->getDescription().c_str());
  }

  if (var->hasFlag(Cvar::Flags::InitOnly)) {
    ImGui::SameLine();
    float size = ImGui::GetFrameHeight();
    // TODO: Define colours in one place
    ImVec4 yellow(1.0, 0.8, 0.0, 1.0);

    ImGui::ImageWithBg(mAlertIcon, ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1),
                /* bg_col= */ ImVec4(0, 0, 0, 0), yellow);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Setting requires a restart to apply.");
    }
  }

  auto valid = var->validatePending();
  if (valid != std::nullopt) {
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", valid->c_str());
  }
}
} // namespace selwonk::core
