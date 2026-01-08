#pragma once
#include "interop.h"

IOP_BEGIN;

struct DebugLine {
  float4 position SLOT(SV_Position);
  float4 color SLOT(Color);
};
#define LINESIZE 32
SIZECHECK(DebugLine, LINESIZE);

// Push constants for the main vertex shader
struct DebugPushConstants {
  float4x4 viewProjection;
  uint64_t vertexBuffer;
};
SIZECHECK(DebugPushConstants, 72);

IOP_END;

#ifndef __cplusplus
struct DebugLineOutput {
  float4 position : SV_Position;
  float4 color : COLOR;
};

struct FragmentShaderOutput {
  float4 color : SV_Target;
};
#endif
