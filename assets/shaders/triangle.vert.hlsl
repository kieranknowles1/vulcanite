#include "triangle.h"

[[vk::binding(0, 0)]]
cbuffer SceneDataCB {
  SceneData sceneData;
};
SamplerState samplers[] : register(s0, space1);
Texture2D textures[] : register(t0, space2);
[[vk::binding(0, 3)]]
StructuredBuffer<Vertex> vertexBuffers[];
[[vk::binding(0, 4)]]
StructuredBuffer<uint> indexBuffers[];

[[vk::binding(0, 5)]]
StructuredBuffer<VertexInstanceData> instanceData;

VertexShaderOutput main(uint vertId : SV_VertexID, uint instanceId : SV_InstanceID) {
  VertexInstanceData instance = instanceData[instanceId];

#ifndef NOINDEX
  uint ib = NonUniformResourceIndex(instance.indexBufferIndex);
  uint index = indexBuffers[ib][vertId];
#else
  uint index = vertId;
#endif
  uint vb = NonUniformResourceIndex(instance.vertexIndex);
  Vertex vtx = vertexBuffers[vb][index];

#ifndef NOMAT
  MaterialData mat = vk::RawBufferLoad<MaterialData>(instance.materialData);
#else
  MaterialData mat;
  mat.colorFactors = float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif

  VertexShaderOutput OUT;
  float4x4 mvp = mul(sceneData.viewProjection, instance.modelMatrix);
  OUT.position = mul(mvp, float4(vtx.position, 1.0f));
  OUT.color = vtx.color * mat.colorFactors;
  OUT.normal = mul(instance.modelMatrix, float4(vtx.normal, 0.0f)).xyz;
  OUT.uv = float2(vtx.uvX, vtx.uvY);
  OUT.instanceId = instanceId;
  return OUT;
}
