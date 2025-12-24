#pragma once

// Interop structs for transferring data between C++ and HLSL
// Declares the following macros:
// - SIZECHECK(ty, exp) - Assert that the size of ty is exp in C++
// - PAD4(id) - Pad 4 bytes on C++ only
// - SLOT(semantic) - Declare a slot with the given semantic, HLSL only

#ifdef __cplusplus
#define SIZECHECK(ty, exp) static_assert(sizeof(ty) == exp, "Size mismatch");
#define SLOT(semantic)
#define PAD4(id) float id;
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
namespace {

using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
using float4x4 = glm::mat4;

} // namespace
#else // HLSL
#define SIZECHECK(ty, exp)
#define SLOT(semantic) : semantic
#define PAD4(id)
#endif

#ifdef __cplusplus
namespace interop {
#endif

struct GradientPushConstants {
  float4 leftColor;
  float4 rightColor;
};
SIZECHECK(GradientPushConstants, 32)

struct Vertex {
  float3 position SLOT(SV_Position);
  PAD4(posPad);
  float4 color SLOT(Color);
  float3 normal SLOT(Normal);
  PAD4(normPad);
  float2 uv SLOT(TexCoord);
}; // struct Vertex
SIZECHECK(Vertex, 56);

struct VertexPushConstants {
  float4x4 viewProjection;
};

#undef SIZECHECK
#undef PAD4
#undef SLOT

#ifdef __cplusplus
} // namespace interop
#endif
