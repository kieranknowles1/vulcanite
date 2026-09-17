#include "profiler.hpp"

#include <algorithm>
#include <cassert>
#include <spdlog/spdlog.h>
#include <string_view>
#include <tracy/Tracy.hpp>

namespace selwonk::core {

Profiler::Profiler() {
#ifdef TRACY_ENABLE
  SPDLOG_INFO("Engine built with profiling support");
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

} // namespace selwonk::core
