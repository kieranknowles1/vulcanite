#include "triangle.h.hlsl"
#include "interop.h"

// TODO: Bindless texturing
// TODO: Standardise descriptor layout
Texture2D texture : register(t0, space1);
SamplerState defaultSampler : register(s0, space1);

SceneData sceneData : register(b0);

FragmentShaderOutput main(VertexShaderOutput IN) {
  FragmentShaderOutput OUT;

  float lightFactor = dot(IN.normal, normalize(sceneData.sunDirection));
  float4 lightColor = float4(lerp(sceneData.ambientColor, sceneData.sunColor, lightFactor), 1.0f);
  float4 sample = texture.Sample(defaultSampler, IN.uv);

  OUT.color = sample * lightColor * IN.color;
  return OUT;
}
