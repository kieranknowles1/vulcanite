#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/norm.hpp>
#include <iomanip>
#include <sstream>
#include <string>

namespace selwonk::core::math {
inline glm::vec3 extractScaleSquared(const glm::mat4& mat) {
  return glm::vec3(glm::length2(glm::vec3(mat[0])),
                   glm::length2(glm::vec3(mat[1])),
                   glm::length2(glm::vec3(mat[2])));
}

inline float maxScale(const glm::mat4& mat) {
  auto square = extractScaleSquared(mat);
  float maxSquared = glm::max(square.x, square.y, square.z);
  return std::sqrt(maxSquared);
}

// TODO: Move to generic util header
inline std::string formatFilesize(size_t bytes) {
  std::array<std::string_view, 4> suffixes = {"B", "KB", "MB", "GB"};
  int index = 0;
  double size = bytes;
  while (size >= 1024 && index < suffixes.size()) {
    size /= 1024.0;
    index++;
  }
  std::ostringstream out;
  out << std::fixed << std::setprecision(2) << size << suffixes[index];
  return out.str();
}
} // namespace selwonk::core::math
