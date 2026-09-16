#include "debug.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace selwonk::assets {

void Debug::drawBox(glm::vec3 origin, glm::vec3 halfExtent, glm::vec4 color) {
  glm::vec3 corner = origin - halfExtent;
  glm::vec3 size = halfExtent * 2.0f;

  const static constexpr std::array<std::pair<glm::vec3, glm::vec3>, 12>
      unitCube{{
          // Top
          {{1, 1, 1}, {0, 1, 1}},
          {{1, 1, 0}, {0, 1, 0}},
          {{1, 1, 1}, {1, 1, 0}},
          {{0, 1, 1}, {0, 1, 0}},

          // Bottom
          {{1, 0, 1}, {0, 0, 1}},
          {{1, 0, 0}, {0, 0, 0}},
          {{1, 0, 1}, {1, 0, 0}},
          {{0, 0, 1}, {0, 0, 0}},

          // Sides
          {{1, 1, 1}, {1, 0, 1}},
          {{0, 1, 1}, {0, 0, 1}},
          {{0, 1, 0}, {0, 0, 0}},
          {{1, 1, 0}, {1, 0, 0}},
      }};

  for (const auto& line : unitCube) {
    auto start = corner + (line.first * size);
    auto end = corner + (line.second * size);
    drawLine({start, end, color});
  }
}

void Debug::drawAxisLines(glm::vec3 position, float length) {
  // X
  drawLine({position, position + glm::vec3(length, 0, 0), Red});
  // Y
  drawLine({position, position + glm::vec3(0, length, 0), Green});
  // Z
  drawLine({position, position + glm::vec3(0, 0, length), Blue});
}

void Debug::drawSphere(glm::vec3 origin, float radius, glm::vec4 color,
                       int resolution) {
  auto angle = [&](int index) {
    // int capI = index % resolution;
    return glm::two_pi<float>() * ((float)index / (float)resolution);
  };
  auto rotate = [](glm::vec2 point, float angle) {
    auto cosTheta = std::cos(angle);
    auto sinTheta = std::sin(angle);

    return glm::vec2((point.x * cosTheta - point.y * sinTheta),
                     (point.y * cosTheta + point.x * sinTheta));
  };

  auto ringPos = [&](int index) {
    glm::vec2 point(radius, 0);
    auto currAng = angle(index);
    auto prevAng = angle(index - 1);
    return std::make_pair(rotate(point, currAng), rotate(point, prevAng));
  };

  for (int i = 0; i < resolution; i++) {
    auto positions = ringPos(i);

    // vertical x-aligned
    drawLine({
        glm::vec3(0, positions.first.x, positions.first.y) + origin,
        glm::vec3(0, positions.second.x, positions.second.y) + origin,
        color,
    });

    // horizontal
    drawLine({
        glm::vec3(positions.first.x, 0, positions.first.y) + origin,
        glm::vec3(positions.second.x, 0, positions.second.y) + origin,
        color,
    });

    // vertical z-aligned
    drawLine({
        glm::vec3(positions.first.x, positions.first.y, 0) + origin,
        glm::vec3(positions.second.x, positions.second.y, 0) + origin,
        color,
    });
  }
}

} // namespace selwonk::vulkan
