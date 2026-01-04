#include "triangle.h.hlsl"
#include "interop.h"

[[vk::push_constant]]
VertexPushConstants pushConstants;

SceneData sceneData : register(b0);

VertexShaderOutput main(uint vertId : SV_VertexID) {
  Vertex vtx = vk::RawBufferLoad<Vertex>(pushConstants.vertexBuffer + (vertId * VERTEXSIZE));

  VertexShaderOutput OUT;
  float4x4 mvp = mul(sceneData.viewProjection, pushConstants.modelMatrix);
  OUT.position = mul(mvp, float4(vtx.position, 1.0f));
  OUT.color = vtx.color;
  OUT.normal = mul(pushConstants.modelMatrix, float4(vtx.normal, 0.0f)).xyz;
  OUT.uv = float2(vtx.uvX, vtx.uvY);
  return OUT;
}
