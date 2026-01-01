#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

namespace selwonk::ecs {
// Lightweight reference to an entity managed by the registry. Does nothing
// without components or systems.
class EntityRef {
public:
  using Id = uint32_t;

  EntityRef() : mId(InvalidId) {}
  EntityRef(Id id) : mId(id) {}

  Id id() const {
    assert(valid());
    return mId;
  }
  bool valid() const { return mId != InvalidId; }

private:
  const static constexpr Id InvalidId = std::numeric_limits<Id>::max();
  Id mId;
};
} // namespace selwonk::ecs
