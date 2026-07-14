#pragma once

#include <array>

#include "math.hpp"

namespace selwonk::core {

// Sphere bounds, defined as an origin and a radius
struct Bounds {
  glm::vec3 origin;
  float radius;

  const Bounds operator*(const glm::mat4& transform) const {
    auto scale = math::maxScale(transform);
    return Bounds{
        // Don't need to transform origin as it's already been transformed
        // prior to frustum cull by the render system
        // .origin = glm::vec3(glm::vec4(origin, 1.0) * transform),
        .origin = origin,
        .radius = radius * scale,
    };
  }
};

class Plane {
public:
  Plane() : normal(glm::vec3(0, 1, 0)), distance(0) {}
  Plane(const glm::vec3& normal, float distance, bool normalise = false);
  ~Plane() {}

  void setNormal(const glm::vec3& norm) { normal = norm; }
  const glm::vec3& getNormal() const { return normal; }

  void setDistance(float dist) { distance = dist; }
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
  bool inFrustum(const glm::mat4& transform, const Bounds& n) const;

protected:
  std::array<Plane, 6> planes;
};
} // namespace selwonk::core
