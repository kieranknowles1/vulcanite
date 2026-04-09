#pragma once

#include "fastgltf/types.hpp"
#include <cstdint>

namespace selwonk::assets {
class ImageBase {
public:
  struct ImgData {
    uint32_t width;
    uint32_t height;
    const unsigned char* data;

    static ImgData visitDataSrc(const fastgltf::Asset& asset,
                                const fastgltf::DataSource& data);
    static ImgData loadFromMemory(const std::byte* bytes, int size);
  };

private:
};
} // namespace selwonk::assets
