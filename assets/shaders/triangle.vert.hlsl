#include "triangle.h.hlsl"
#include "interop.h"

[[vk::push_constant]]
VertexPushConstants pushConstants;

VertexShaderOutput main(Vertex vtx) {
  VertexShaderOutput OUT;
  OUT.position = float4(vtx.position, 1.0f);
  OUT.color = vtx.color;
  return OUT;
}
