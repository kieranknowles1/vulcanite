#include "triangle.h"

SceneData sceneData : register(b0, space0);

SamplerState samplers[] : register(s0, space1);
Texture2D textures[] : register(t0, space2);

[[vk::binding(0, 5)]]
StructuredBuffer<VertexInstanceData> instanceData;

// TODO: Bindless texturing
// TODO: Standardise descriptor layout

FragmentShaderOutput main(VertexShaderOutput IN) {
  VertexInstanceData instance = instanceData[IN.instanceId];

  FragmentShaderOutput OUT;
  SamplerState s = samplers[NonUniformResourceIndex(instance.samplerIndex)];
  Texture2D texture = textures[NonUniformResourceIndex(instance.textureIndex)];

  float lightFactor = dot(IN.normal, normalize(sceneData.sunDirection));
  float4 lightColor = float4(lerp(sceneData.ambientColor, sceneData.sunColor, lightFactor), 1.0f);
  float4 sample = texture.Sample(s, IN.uv);

  OUT.color = sample * lightColor * IN.color;
  // FIXME: Current scene uses RGB channels for alpha
  // OUT.color.a = sample.a;
  OUT.color.a = (sample.r + sample.g + sample.b) / 3.0;
  return OUT;
}
