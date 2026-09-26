#include "sceneviewui.hpp"
#include <imgui.h>

namespace selwonk::ui {
void SceneViewUi::drawImpl()
{
  // TODO: Optional version of forEach
  // TODO: display everything in a tree, show parents
  // TODO: selection for debug draw
  // TODO: Virtual scroll. Need partial forEach
  if (ImGui::BeginTable("Scene", 4,
    ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
    ImGuiTableFlags_SizingFixedFit)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Translation");
    ImGui::TableSetupColumn("Rotation");
    ImGui::TableSetupColumn("Scale");
    ImGui::TableHeadersRow();
    mEcs.forEach<const ecs::Named&, const ecs::Transform&>(
      [&](auto entity, auto name, auto tfm) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", name.mName.c_str());
        ImGui::TableNextColumn();
        // TODO: Reusable toString or something for transform
        ImGui::Text("%.1f,%.1f,%.1f", tfm.mTranslation.x,
          tfm.mTranslation.y, tfm.mTranslation.z);
        ImGui::TableNextColumn();
        auto euler = glm::degrees(glm::eulerAngles(tfm.mRotation));
        ImGui::Text("%.0f,%.0f,%.0f", euler.x, euler.y, euler.z);
        ImGui::TableNextColumn();
        ImGui::Text("%.2f,%.2f,%.2f", tfm.mScale.x, tfm.mScale.y,
          tfm.mScale.z);
      });
    ImGui::EndTable();
  }
}
}