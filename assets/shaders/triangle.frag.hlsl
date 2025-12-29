#include "triangle.h.hlsl"

// TODO: Bindless texturing
// TODO: Standardise descriptor layout
Texture2D texture : register(t0, space1);
SamplerState sample : register(s0, space1);

FragmentShaderOutput main(VertexShaderOutput IN) {
  FragmentShaderOutput OUT;
  OUT.color = /*IN.color **/ texture.Sample(sample, IN.uv);
  return OUT;
}
