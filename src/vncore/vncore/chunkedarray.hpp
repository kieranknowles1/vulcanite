#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

namespace selwonk::core {
// Densely packed array of objects stored in large contiguous chunks
// Attempting to read a non-existent element is undefined
template <typename T, size_t ChunkSize = 1024> class ChunkedArray {
public:
  void insert(size_t i, const T& value) {
    Chunk& c = getChunk(i);
    size_t idx = chunkIdx(i);
    c[idx] = value;
  }
  T& get(size_t i) {
    Chunk& c = getChunk(i);
    size_t idx = chunkIdx(i);
    return c[idx];
  }

  T& operator[](size_t i) { return get(i); }

  // Get the number of entries allocated
  size_t capacity() const { return mChunks.size() * ChunkSize; }

private:
  using Chunk = std::array<T, ChunkSize>;
  std::vector<std::unique_ptr<Chunk>> mChunks;

  // Get the chunk index an entry belongs in
  Chunk& getChunk(size_t i) {
    auto idx = i / ChunkSize;
    if (mChunks.size() <= idx)
      mChunks.resize(idx + 1);
    if (mChunks[idx] == nullptr)
      mChunks[idx] = std::make_unique<Chunk>();
    return *(mChunks[idx].get());
  }
  // Get an entry's position within its chunk
  size_t chunkIdx(size_t i) { return i % ChunkSize; }
};
} // namespace selwonk::core
