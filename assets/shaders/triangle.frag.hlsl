#include "triangle.h.hlsl"

FragmentShaderOutput main(VertexShaderOutput IN) {
  FragmentShaderOutput OUT;
  OUT.color = IN.color;
  return OUT;
}
