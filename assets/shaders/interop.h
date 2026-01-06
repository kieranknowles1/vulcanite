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

// Push constants for a background gradient compute shader
struct GradientPushConstants {
  float4 leftColor;
  float4 rightColor;
};
SIZECHECK(GradientPushConstants, 32)

// Per-vertex data, consumed by the vertex shader. As we use a bindless
// renderer, there is no interaction with the fixed-function hardware here
struct Vertex {
  float3 position SLOT(SV_Position);
  float uvX;
  float4 color SLOT(Color);
  float3 normal SLOT(Normal);
  float uvY;
}; // struct Vertex
#define VERTEXSIZE 48
SIZECHECK(Vertex, VERTEXSIZE);

// Push constants for the main vertex shader
struct VertexPushConstants {
  float4x4 modelMatrix;
  uint64_t indexBuffer;
  uint64_t vertexBuffer;
  uint64_t materialData;
  uint samplerIndex;
  PAD4(padSampler)
};
SIZECHECK(VertexPushConstants, 96);

// Per-material data
struct MaterialData {
  float4 colorFactors;
  float4 metalRoughnessFactors;
};

// Scene-level data for vertex/fragment uniform buffers
struct SceneData {
  float4x4 viewProjection;
  float3 ambientColor;
  PAD4(ambColorPad);
  float3 sunDirection;
  PAD4(sunDirPad);
  float3 sunColor;
  PAD4(sunColorPad);
};
SIZECHECK(SceneData, 112)

#undef SIZECHECK
#undef PAD4
#undef SLOT

#ifdef __cplusplus
} // namespace interop
#endif
