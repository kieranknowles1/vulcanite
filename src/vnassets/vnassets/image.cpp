#include "image.hpp"

#include <spdlog/spdlog.h>
#include <stb_image.h>

namespace selwonk::assets {

ImageBase::ImgData
ImageBase::ImgData::loadFromAsset(const fastgltf::Asset& asset,
                                 const fastgltf::DataSource& data) {
  return std::visit(
      fastgltf::visitor{
          [](auto& data) -> ImageBase::ImgData {
            throw std::runtime_error("Unsupported image type. Got " +
                                     std::string(typeid(data).name()));
          },
          [&](const fastgltf::sources::Array& array) {
            return loadFromMemory(array.bytes.data(), array.bytes.size_bytes());
          },
          [&](const fastgltf::sources::BufferView& view) {
            auto& bufferView = asset.bufferViews[view.bufferViewIndex];
            auto& buffer = asset.buffers[bufferView.bufferIndex];

            auto bytes = std::visit(
                fastgltf::visitor{
                    [](auto& val) -> const std::byte* {
                      throw std::runtime_error("Unsupported buffer type");
                    },
                    [&](const fastgltf::sources::Array& array) {
                      return array.bytes.data() + bufferView.byteOffset;
                    }},
                buffer.data);
            return loadFromMemory(bytes, bufferView.byteLength);
          }},
      data);
}

ImageBase::ImgData ImageBase::ImgData::loadFromMemory(const std::byte* bytes,
                                                      int size) {
  int width;
  int height;
  int channels; // stb_image converts for us, can ignore value
  // TODO: Could we load/upload fewer channels if the image has fewer?
  auto data =
      stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bytes), size,
                            &width, &height, &channels, 4);
  if (data == nullptr) {
    spdlog::error("Failed to load image: {}", stbi_failure_reason());
  }

  return ImgData{static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                 data};
}

ImageBase::ImgData::~ImgData() {
  stbi_image_free((void*)data);
}

} // namespace selwonk::assets
