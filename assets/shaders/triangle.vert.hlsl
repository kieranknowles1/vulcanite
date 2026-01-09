#include "triangle.h"

[[vk::push_constant]]
VertexPushConstants pushConstants;

SceneData sceneData : register(b0, space0);

VertexShaderOutput main(uint vertId : SV_VertexID) {
#ifndef NOINDEX
  uint index = vk::RawBufferLoad<uint>(pushConstants.indexBuffer + (vertId * 4));
#else
  uint index = vertId;
#endif
  Vertex vtx = vk::RawBufferLoad<Vertex>(pushConstants.vertexBuffer + (index * VERTEXSIZE));

#ifndef NOMAT
  MaterialData mat = vk::RawBufferLoad<MaterialData>(pushConstants.materialData);
#else
  MaterialData mat;
  mat.colorFactors = float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif

  VertexShaderOutput OUT;
  float4x4 mvp = mul(sceneData.viewProjection, pushConstants.modelMatrix);
  OUT.position = mul(mvp, float4(vtx.position, 1.0f));
  OUT.color = vtx.color * mat.colorFactors;
  OUT.normal = mul(pushConstants.modelMatrix, float4(vtx.normal, 0.0f)).xyz;
  OUT.uv = float2(vtx.uvX, vtx.uvY);
  return OUT;
}
