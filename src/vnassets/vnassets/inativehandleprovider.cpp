#include "inativehandleprovider.hpp"

namespace selwonk::assets {
ImageBase::GuiHandle INativeHandleProvider::addGuiTexture(ImageBase::Handle handle)
{
  auto id = registerGuiTexture(handle);
  // Don't inc handle since we now own it
  return mGuiTextures.insert(id, handle);
}

bool INativeHandleProvider::decRef(ImageBase::GuiHandle handle)
{
  auto id = mGuiTextures.get(handle);
  bool freed = mGuiTextures.decRef(handle);
  if (freed) {
    freeGuiTexture(id.imgui);
    // We no longer own the underlying texture
    decRef(id.texture);
  }
  return freed;
}

}