#pragma once
#include "interop.h"

IOP_BEGIN;

// Push constants for a background gradient compute shader
struct GradientPushConstants {
  float4 leftColor;
  float4 rightColor;
};
SIZECHECK(GradientPushConstants, 32)

IOP_END;
