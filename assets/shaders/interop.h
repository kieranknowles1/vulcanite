#pragma once

// Interop structs for transferring data between C++ and HLSL

#ifdef __cplusplus
#include <glm/vec4.hpp>
namespace {
using float4 = glm::vec4;
}
#else // HLSL
#define ASSERT_SIZE
#endif

struct GradientPushConstants {
  float4 topLeftColor;
  float4 bottomRightColor;
};
