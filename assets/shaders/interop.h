#pragma once

// Interop structs for transferring data between C++ and HLSL

#ifdef __cplusplus
#include <glm/vec4.hpp>
namespace {
using float4 = glm::vec4;
}
#else // HLSL

#endif

struct GradientPushConstants {
  float4 leftColor;
  float4 rightColor;
};
