#pragma once

#include <bitset>
#include <cstdint>
#include <fmt/base.h>
#include <memory>
#include <vector>

#include "entity.hpp"

namespace selwonk::ecs {
enum class ComponentType : uint8_t {
  Transform,
  Named,
  Renderable,
  Max,
};

enum class EntityFlag : uint8_t {
  // Entity exists and has not been deleted
  // Deleted entities can not be resurrected
  // Accessing any further information about a deleted entity is undefined
  // behaviour
  Alive,
  // Entity is currently active and processed by all systems
  // Systems can opt-in to process disabled entities
  // All entities are enabled by default, and may be disabled/re-enabled at any
  // time
  Enabled,
  Max,
};

class ComponentMask {
public:
  // A mask representing a non-existent entity. Importantly, this does not have
  // the `Alive` flag set.
  consteval static ComponentMask null() { return ComponentMask(); }

  constexpr bool hasComponent(ComponentType type) const {
    return mData[componentIndex(type)];
  }
  constexpr void setComponentPresent(ComponentType type, bool value) {
    mData[componentIndex(type)] = value;
  }

  constexpr bool hasFlag(EntityFlag flag) const {
    return mData[flagIndex(flag)];
  }

  constexpr void setFlag(EntityFlag flag, bool value) {
    mData[flagIndex(flag)] = value;
  }

  // Does this mask have all components and flags as the other mask?
  constexpr bool matches(const ComponentMask& mask) const {
    return (mData & mask.mData) == mask.mData;
  }

private:
  constexpr size_t flagIndex(EntityFlag flag) const {
    return FlagStart + static_cast<size_t>(flag);
  }
  constexpr size_t componentIndex(ComponentType type) const {
    return ComponentStart + static_cast<size_t>(type);
  }

  // Total number of component types
  const static constexpr size_t ComponentStart = 0;
  const static constexpr size_t ComponentCount =
      static_cast<size_t>(ComponentType::Max);
  const static constexpr size_t FlagStart = ComponentCount;
  const static constexpr size_t FlagCount =
      static_cast<size_t>(EntityFlag::Max);

  std::bitset<ComponentCount + FlagCount> mData;
};

template <typename T, size_t ChunkSize = 1024> class ComponentArray {
public:
  using ValueType = T;
  const char* getTypeName() const { return T::Name; }

  void add(EntityRef entity, const T& value) {
    Chunk& c = getChunk(entity);
    size_t idx = chunkIdx(entity);
    c[idx] = value;

#ifdef VN_LOGCOMPONENTSTATS
    mSize++;
#endif
  }
  T& get(EntityRef entity) {
    Chunk& c = getChunk(entity);
    size_t idx = chunkIdx(entity);
    return c[idx];
  }

#ifdef VN_LOGCOMPONENTSTATS
  // Get the number of components of this type
  size_t size() const { return mSize; }

  // Get the number of components allocated
  size_t capacity() const { return mChunks.size() * ChunkSize; }

#endif

private:
  using Chunk = std::array<T, ChunkSize>;
  std::vector<std::unique_ptr<Chunk>> mChunks;

  // Get the chunk index an entity belongs in
  Chunk& getChunk(EntityRef ent) {
    auto idx = ent.id() / ChunkSize;
    if (mChunks.size() <= idx)
      mChunks.resize(idx + 1);
    if (mChunks[idx] == nullptr)
      mChunks[idx] = std::make_unique<Chunk>();
    return *(mChunks[idx].get());
  }
  // Get an entity's position within its chunk
  size_t chunkIdx(EntityRef ent) { return ent.id() % ChunkSize; }

#ifdef VN_LOGCOMPONENTSTATS
  size_t mSize = 0;
#endif
};

} // namespace selwonk::ecs
