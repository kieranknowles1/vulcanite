#pragma once
#include "interop.h"

IOP_BEGIN;

// Push constants for a background gradient compute shader
IOP_STRUCT(GradientPushConstants, 32, {
  float4 leftColor;
  float4 rightColor;
});

IOP_END;
