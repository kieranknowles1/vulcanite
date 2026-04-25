#include "profiler.hpp"

#include <algorithm>
#include <cassert>
#include <fmt/base.h>
// TODO: Move IMGui to engine
#include <imgui.h>
#include <string_view>
#include <tracy/Tracy.hpp>

namespace selwonk::core {

Profiler::Profiler() {
#ifdef TRACY_ENABLE
  fmt::println("Engine built with profiling support");
#endif
}

Profiler::Section* Profiler::Section::getOrAdd(std::string_view name) {
  auto nameMatches = [name](auto& s) -> bool { return s->mName == name; };

  const auto existing =
      std::find_if(mChildren.begin(), mChildren.end(), nameMatches);
  if (existing == mChildren.end()) {
    mChildren.emplace_back(std::make_unique<Section>(name, this));
    return mChildren.back().get();
  }
  return existing->get();
}

void Profiler::pushSection(std::string_view name) {
  auto section = mCurrentSection->getOrAdd(name);
  section->begin();
  mCurrentSection = section;
}

void Profiler::popSection() {
  mCurrentSection->end();

  mCurrentSection = mCurrentSection->mParent;
}

void Profiler::beginFrame() {
  assert(mCurrentSection == &mRootSection && "Dangling section");
  mRootSection.begin();
  FrameMark;
}

// Record timing for the last section
void Profiler::endFrame() { mRootSection.end(); }

void Profiler::printSectionTimes(const Section& section) {
  int flags = ImGuiTreeNodeFlags_DefaultOpen;
  if (section.mChildren.empty())
    flags |= ImGuiTreeNodeFlags_Leaf;

  if (!ImGui::TreeNodeEx(section.mName.c_str(), flags))
    return;

  ImGui::SameLine();
  ImGui::Text("%.3fms", section.timeMs());

  for (auto& child : section.mChildren) {
    printSectionTimes(*child);
  }

  ImGui::TreePop();
}

void Profiler::printTimes() {
  if (ImGui::Begin("Metrics")) {
    ImGui::LabelText("Culled/Total", "%d/%d", mExtraMetrics.drawnRenderable,
                     mExtraMetrics.totalRenderable);
    ImGui::LabelText("Transparent Surfaces", "%d",
                     mExtraMetrics.transparentRenderable);

    printSectionTimes(mRootSection);

    // auto us = std::chrono::duration_cast<std::chrono::microseconds>(total);
    float ms = mRootSection.timeMs();
    ImGui::LabelText("Total/Target", "%.3fms/%.3fms", ms, 1000.0f / 144.0f);

    auto framerate = 1000.0f / ms;
    ImGui::LabelText("Framerate", "%.0ffps", framerate);
  }
  ImGui::End();
}
} // namespace selwonk::core
