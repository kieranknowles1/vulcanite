#include "triangle.h"

SceneData sceneData : register(b0, space0);

SamplerState samplers[] : register(s0, space1);
Texture2D textures[] : register(t0, space2);

[[vk::binding(0, 5)]]
StructuredBuffer<VertexPushConstants> instanceData;

// TODO: Bindless texturing
// TODO: Standardise descriptor layout

FragmentShaderOutput main(VertexShaderOutput IN) {
  VertexPushConstants instance = instanceData[IN.instanceId];

  FragmentShaderOutput OUT;
  SamplerState s = samplers[NonUniformResourceIndex(instance.samplerIndex)];
  Texture2D texture = textures[NonUniformResourceIndex(instance.textureIndex)];

  float lightFactor = dot(IN.normal, normalize(sceneData.sunDirection));
  float4 lightColor = float4(lerp(sceneData.ambientColor, sceneData.sunColor, lightFactor), 1.0f);
  float4 sample = texture.Sample(s, IN.uv);

  OUT.color = sample * lightColor * IN.color;
  return OUT;
}
