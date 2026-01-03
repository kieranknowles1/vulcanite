#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <vector>

#include "entity.hpp"

namespace selwonk::ecs {
enum class ComponentType : uint8_t {
  // Entity exists and has not been deleted
  // Deleted entities can not be resurrected
  Alive,
  // Entity is currently active and processed by all systems
  // Systems can opt-in to process disabled entities
  // All entities are enabled by default, and may be disabled/re-enabled at any
  // time
  Enabled,
  Transform,
  Named,
  Renderable,
  Max,
};

// Total number of component types
const static constexpr std::underlying_type_t<ComponentType> ComponentCount =
    static_cast<std::underlying_type_t<ComponentType>>(ComponentType::Max);

// Mask that can hold present/absent for each component type
using ComponentMask = std::bitset<ComponentCount>;

// A mask representing a non-existent entity
const static constexpr ComponentMask NullMask = ComponentMask(0);

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
