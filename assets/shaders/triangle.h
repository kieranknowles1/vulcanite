#pragma once
#include "interop.h"

IOP_BEGIN;

// Per-vertex data, consumed by the vertex shader. As we use a bindless
// renderer, there is no interaction with the fixed-function hardware here
struct Vertex {
  float3 position SLOT(SV_Position);
  float uvX;
  float4 color SLOT(Color);
  float3 normal SLOT(Normal);
  float uvY;
}; // struct Vertex
SIZECHECK(Vertex, 48);

// Push constants for the main vertex shader
struct VertexPushConstants {
  float4x4 modelMatrix;
  uint64_t materialData;
  uint indexBufferIndex;
  uint textureIndex;
  uint samplerIndex;
  uint vertexIndex;
};
SIZECHECK(VertexPushConstants, 88);

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

IOP_END;

#ifndef __cplusplus
struct VertexShaderOutput {
  float4 position : SV_Position;
  float4 color : COLOR;
  float2 uv : TEXCOORD0;
  float3 normal : NORMAL;
};

struct FragmentShaderOutput {
  float4 color : SV_Target;
};
#endif
