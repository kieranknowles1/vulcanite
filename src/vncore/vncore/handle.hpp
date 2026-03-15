#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

namespace selwonk::core {
// TODO: Ref counting for all users, not just HandleList
// Generic object handle with generation tracking
// `Tag` is used for type safety, declare `using Handle = Handle<MyContainer>`
// for easy use by downstream consumers. A MeshArray::Handle can't be used to
// index a TextureArray::Handle
template <typename Tag> class Handle {
public:
  // Layout:
  // i: index
  // g: generation
  // gggggggg-iiiiiiii-iiiiiiii-iiiiiiii
  // All ones represents an invalid handle
  using Backing = uint32_t;
  using GenerationBacking = uint8_t;

  const static constexpr int IndexBits = 24;
  const static constexpr int GenerationBits =
      std::numeric_limits<Backing>::digits - IndexBits;
  static_assert(GenerationBits <=
                    std::numeric_limits<GenerationBacking>::digits,
                "GenerationBacking cannot hold generation number");

  const static constexpr Backing InvalidValue =
      std::numeric_limits<Backing>::max();

  const static constexpr Backing GenerationMask = InvalidValue << IndexBits;
  const static constexpr Backing IndexMask = InvalidValue >> GenerationBits;

  static_assert((GenerationMask & IndexMask) == 0,
                "Generation and index overlap");
  static_assert((GenerationMask | IndexMask) == InvalidValue,
                "Unused bits in handle");

  explicit Handle(Backing v, GenerationBacking gen)
      : mValue(v | (gen << IndexBits)) {
    assert(generation() == gen);
    assert(value() == v);
  }
  Handle() : mValue(InvalidValue) {}
  constexpr Backing value() const {
    assert(valid());
    return mValue & IndexMask;
  }
  constexpr GenerationBacking generation() const {
    assert(valid());
    return (mValue & GenerationMask) >> IndexBits;
  }
  constexpr bool valid() const { return mValue != InvalidValue; }

private:
  Backing mValue;
};
} // namespace selwonk::core
