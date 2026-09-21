#pragma once
#include <string_view>

namespace selwonk::ui {
  class Element {
public:
  virtual ~Element() = default;

  virtual const char* name() const = 0;

  void draw();
  bool visible() const { return mVisible; }

protected:
  // Draw element, runs inside an ImGui::Begin context
  virtual void drawImpl() = 0;

private:
  bool mVisible = true;
};
}
