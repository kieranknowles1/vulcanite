#pragma once

#include <glm/glm.hpp>

#include "mesh.hpp"

namespace selwonk::vulkan {

class Plane {
public:
  Plane() : normal(glm::vec3(0, 1, 0)), distance(0) {}
  Plane(const glm::vec3& normal, float distance, bool normalise = false);
  ~Plane() {}

  void setNormal(const glm::vec3& normal) { this->normal = normal; }
  const glm::vec3& getNormal() const { return normal; }

  void setDistance(float distance) { this->distance = distance; }
  float getDistance() const { return distance; }

  bool sphereInPlane(const glm::vec3& position, float radius) const;

protected:
  glm::vec3 normal;
  float distance;
};

class Frustum {
public:
  Frustum() {}
  Frustum(const glm::mat4& viewProj) { fillFromMatrix(viewProj); }
  ~Frustum() {}

  // Fill the frustum with planes extracted from the view-projection matrix
  void fillFromMatrix(const glm::mat4& viewProj);
  bool inFrustum(const glm::mat4& transform, const Mesh::Bounds& n) const;

protected:
  std::array<Plane, 6> planes;
};
} // namespace selwonk::vulkan
