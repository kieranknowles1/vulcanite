#include "triangle.h.hlsl"
#include "interop.h"

[[vk::push_constant]]
VertexPushConstants pushConstants;

SceneData sceneData : register(b0);

VertexShaderOutput main(Vertex vtx) {
  VertexShaderOutput OUT;
  OUT.position = mul(sceneData.viewProjection, float4(vtx.position, 1.0f));
  OUT.color = vtx.color;
  return OUT;
}
