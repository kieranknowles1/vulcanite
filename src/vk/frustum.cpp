#include "frustum.hpp"

namespace selwonk::vulkan {

Plane::Plane(const glm::vec3& normal, float distance, bool normalise) {
  if (normalise) {
    float length = std::sqrt(glm::dot(normal, normal));
    this->normal = normal / length;
    this->distance = distance / length;
  } else {
    this->normal = normal;
    this->distance = distance;
  }
}

bool Plane::sphereInPlane(const glm::vec3& position, float radius) const {
  if (glm::dot(position, normal) + distance <= -radius) {
    return false;
  }
  return true;
}

bool Frustum::inFrustum(const glm::mat4& transform,
                        const Mesh::Bounds& node) const {
  for (int p = 0; p < 6; p++) {
    if (!planes[p].sphereInPlane(transform * glm::vec4(node.origin, 1.0f),
                                 node.radius)) {
      return false;
    }
  }
  return true;
}

void Frustum::fillFromMatrix(const glm::mat4& matrix) {
  // Copy-paste from nclgl based uni work, hacked to work with glm :)
  const float* values = &matrix[0][0];
  // Extract each axis from the matrix
  // then add/subtract to tilt the normals
  glm::vec3 xaxis = glm::vec3(values[0], values[4], values[8]);
  glm::vec3 yaxis = glm::vec3(values[1], values[5], values[9]);
  glm::vec3 zaxis = glm::vec3(values[2], values[6], values[10]);
  glm::vec3 waxis = glm::vec3(values[3], values[7], values[11]);

  // Right
  planes[0] = Plane(waxis - xaxis, values[15] - values[12], true);

  // Left
  planes[1] = Plane(waxis + xaxis, values[15] + values[12], true);

  // Bottom
  planes[2] = Plane(waxis + yaxis, values[15] + values[13], true);

  // Top
  planes[3] = Plane(waxis - yaxis, values[15] - values[13], true);

  // Far
  planes[4] = Plane(waxis - zaxis, values[15] - values[14], true);

  // Near
  planes[5] = Plane(waxis + zaxis, values[15] + values[14], true);
}

} // namespace selwonk::vulkan
