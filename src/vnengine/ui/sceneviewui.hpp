#pragma once

#include <vnecs/registry.hpp>

#include "element.hpp"

namespace selwonk::ui {
class SceneViewUi final : public Element {
public:
SceneViewUi(const ecs::Registry& ecs)
  : mEcs(ecs) {}
~SceneViewUi() = default;

const char* name() const { return "Scene Nodes"; }
void drawImpl() override;

private:
  const ecs::Registry& mEcs;
};
}
