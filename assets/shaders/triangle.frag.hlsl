#include "triangle.h"

[[vk::push_constant]]
VertexPushConstants pushConstants;

SceneData sceneData : register(b0, space0);

SamplerState samplers[] : register(s0, space1);
Texture2D textures[] : register(t0, space2);

// TODO: Bindless texturing
// TODO: Standardise descriptor layout

FragmentShaderOutput main(VertexShaderOutput IN) {
  FragmentShaderOutput OUT;
  SamplerState s = samplers[NonUniformResourceIndex(pushConstants.samplerIndex)];
  Texture2D texture = textures[NonUniformResourceIndex(pushConstants.textureIndex)];

  float lightFactor = dot(IN.normal, normalize(sceneData.sunDirection));
  float4 lightColor = float4(lerp(sceneData.ambientColor, sceneData.sunColor, lightFactor), 1.0f);
  float4 sample = texture.Sample(s, IN.uv);

  OUT.color = sample * lightColor * IN.color;
  return OUT;
}
