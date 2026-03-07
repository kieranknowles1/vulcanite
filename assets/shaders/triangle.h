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

// Per instance data for the main vertex shader
struct VertexInstanceData {
  VkDrawIndirectCommand drawData;

  float4x4 modelMatrix;
  uint64_t materialData;
  uint indexBufferIndex;
  uint textureIndex;
  uint samplerIndex;
  uint vertexIndex;
  PAD(8, padding);
};
SIZECHECK(VertexInstanceData, 112);

// Per-material data
struct MaterialData {
  float4 colorFactors;
  float4 metalRoughnessFactors;
};

// Scene-level data for vertex/fragment uniform buffers
struct SceneData {
  float4x4 viewProjection;
  float3 ambientColor;
  PAD(4, ambColorPad);
  float3 sunDirection;
  PAD(4, sunDirPad);
  float3 sunColor;
  PAD(4, sunColorPad);
};
SIZECHECK(SceneData, 112)

IOP_END;

#ifndef __cplusplus
struct VertexShaderOutput {
  float4 position : SV_Position;
  float4 color : COLOR;
  float2 uv : TEXCOORD0;
  float3 normal : NORMAL;
  // TODO: Can this be done without passing an output from vtx?
  // FIXME: [vk:warning (performance)] WARNING-Shader-OutputNotConsumed:
  // vkCreateGraphicsPipelines(): pCreateInfos[0] (SPIR-V Interface)
  // VK_SHADER_STAGE_VERTEX_BIT declared to output location 3 Component 0 but is
  // not an Input declared by VK_SHADER_STAGE_FRAGMENT_BIT.
  uint instanceId : INSTANCE;
};

struct FragmentShaderOutput {
  float4 color : SV_Target;
};
#endif
