#include "element.hpp"

#include <imgui.h>

namespace selwonk::ui {
void Element::draw()
{
  if (!visible()) return;

  if (ImGui::Begin(name(), &mVisible)) {
    drawImpl();
  }
  ImGui::End();
}
}
