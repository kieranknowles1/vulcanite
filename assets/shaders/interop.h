#pragma once

// Interop structs for transferring data between C++ and HLSL
// Declares the following macros:
// - SIZECHECK(ty, exp) - Assert that the size of ty is exp in C++
// - PAD4(id) - Pad 4 bytes on C++ only
// - SLOT(semantic) - Declare a slot with the given semantic, HLSL only
//
// Any shared structs should be defined between IOP_BEGIN and IOP_END markers
// HLSL only should be wrapped in #ifndef __cplusplus

#ifdef __cplusplus
#define SIZECHECK(ty, exp) static_assert(sizeof(ty) == exp, "Size mismatch");
#define SLOT(semantic)
#define PAD4(id) float id;
#define IOP_BEGIN namespace interop {
#define IOP_END }
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
namespace interop {

using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
using float4x4 = glm::mat4;

} // namespace interop
#else // HLSL
#define SIZECHECK(ty, exp)
#define SLOT(semantic) : semantic
#define PAD4(id)
#define IOP_BEGIN
#define IOP_END
#endif
