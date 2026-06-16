#pragma once

// Interop structs for transferring data between C++ and HLSL
// Declares the following macros:
// - IOP_STRUCT(ty, size, decl) - Declare a struct for use between C++ and HLSL.
//   Size and padding must be manually inserted.
// - PAD(bytes, id) - Pad a number of bytes on C++ only
// - SLOT(semantic) - Declare a slot with the given semantic, HLSL only
//
// Any shared structs should be defined between IOP_BEGIN and IOP_END markers
// HLSL only should be wrapped in #ifndef __cplusplus

#ifdef __cplusplus

// HLSL structs and members are aligned to 16 bytes, e.g.
// float2 a; float3 b;
// Will compile to
// float2 a; char[8] padding; float3 b; char[4] padding;
#define IOP_STRUCT(ty, size, decl) \
  struct ty decl; \
  static_assert(sizeof(ty) == size, "Size mismatch"); \
  static_assert(sizeof(ty) % 16 == 0, "Structs must be 16-byte aligned");

#define SLOT(semantic)
#define PAD(bytes, id) char id[bytes];
#define IOP_BEGIN namespace interop {
#define IOP_END }
#include <glm/glm.hpp>
namespace interop {

using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
using float4x4 = glm::mat4;
using uint = uint32_t;

} // namespace interop
#else // HLSL
#define IOP_STRUCT(ty, size, decl) struct ty decl;
#define SLOT(semantic) : semantic
#define PAD(bytes, id)
#define IOP_BEGIN
#define IOP_END

#endif

IOP_BEGIN;

// Copy of VkDrawIndirectCommand to avoid bringing in a vulkan dependency to assets
struct DrawIndirectCommand {
  uint vertexCount;
  uint instanceCount;
  uint firstVertex;
  uint firstInstance;
};

IOP_END;