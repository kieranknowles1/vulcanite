#include "triangle.h.hlsl"

VertexShaderOutput main(uint vertexId : SV_VertexID) {
  const float3 positions[3] = {
    {1.0f, 1.0f, .0f},
    {-1.0f, 1.0f, .0f},
    {0.0f, -1.0f, .0f}
  };
  const float4 colors[3] = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f}
  };

  VertexShaderOutput OUT;
  OUT.position = float4(positions[vertexId], 1.0f);
  OUT.color = colors[vertexId];
  return OUT;
}
