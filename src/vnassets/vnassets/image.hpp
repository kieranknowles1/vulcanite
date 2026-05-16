#pragma once

#include "fastgltf/types.hpp"
#include <cstdint>

namespace selwonk::assets {
class ImageBase {
public:
  // Data associated with a loaded image. Must be freed manually after upload
  class ImgData {
  public:
    ImgData(uint32_t width, uint32_t height, const unsigned char* data)
      : width(width)
      , height(height)
      , data(data) {}
    ~ImgData();

    ImgData(ImgData&) = delete;
    ImgData(ImgData&& other) noexcept
      : width(other.width)
      , height(other.height)
      , data(other.data) {
      other.data = nullptr;
    }

    uint32_t width;
    uint32_t height;
    const unsigned char* data;

    static ImgData loadFromAsset(const fastgltf::Asset& asset,
                                const fastgltf::DataSource& data);
    static ImgData loadFromMemory(const std::byte* bytes, int size);
  };

private:
};
} // namespace selwonk::assets
