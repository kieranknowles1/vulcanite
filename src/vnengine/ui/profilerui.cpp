#include "profilerui.hpp"

#include <imgui.h>

namespace selwonk::ui {
void ProfilerUi::printTimes() {
  auto& metrics = mProfiler.getExtraMetrics();
  if (ImGui::Begin("Metrics")) {
    ImGui::LabelText("Culled/Total", "%d/%d", metrics.drawnRenderable,
      metrics.totalRenderable);
    ImGui::LabelText("Transparent Surfaces", "%d",
      metrics.transparentRenderable);

    auto& root = mProfiler.getRootSection();
    printSectionTimes(root);

    float ms = root.timeMs();
    ImGui::LabelText("Total/Target", "%.3fms/%.3fms", ms, 1000.0f / 144.0f);

    auto framerate = 1000.0f / ms;
    ImGui::LabelText("Framerate", "%.0ffps", framerate);
  }
  ImGui::End();
}

void ProfilerUi::printSectionTimes(const core::Profiler::Section& section) {
  int flags = ImGuiTreeNodeFlags_DefaultOpen;
  if (section.mChildren.empty())
    flags |= ImGuiTreeNodeFlags_Leaf;

  bool expanded = ImGui::TreeNodeEx(section.mName.c_str(), flags);
  ImGui::SameLine();
  ImGui::Text("%.3fms", section.timeMs());

  if (!expanded)
    return;

  for (auto& child : section.mChildren) {
    printSectionTimes(*child);
  }

  ImGui::TreePop();
}

}
