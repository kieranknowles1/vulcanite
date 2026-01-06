#pragma once

#include <cassert>
#include <cstdint>
#include <functional> // For hash
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

  constexpr bool operator==(const EntityRef& rhs) const {
    return mId == rhs.mId;
  }

private:
  const static constexpr Id InvalidId = std::numeric_limits<Id>::max();
  Id mId;
};
} // namespace selwonk::ecs

template <> struct std::hash<selwonk::ecs::EntityRef> {
  size_t operator()(const selwonk::ecs::EntityRef& entity) const {
    return std::hash<selwonk::ecs::EntityRef::Id>{}(entity.id());
  }
};
