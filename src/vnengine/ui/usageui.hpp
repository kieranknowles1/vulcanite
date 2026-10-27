#pragma once

#include <vnassets/inativehandleprovider.hpp>
#include <vnecs/registry.hpp>

#include "element.hpp"

namespace selwonk::ui {
class UsageUi final : public Element {
public:
  UsageUi(const assets::INativeHandleProvider& handles,
          const ecs::Registry& ecs)
      : mHandles(handles), mEcs(ecs) {}
  ~UsageUi() = default;

  const char* name() const override { return "Limits & Usage"; }
  void drawImpl() override;

private:
  const assets::INativeHandleProvider& mHandles;
  const ecs::Registry& mEcs;
};
} // namespace selwonk::ui
