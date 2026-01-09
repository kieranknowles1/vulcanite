#include "triangle.h"

FragmentShaderOutput main(VertexShaderOutput IN) {
  FragmentShaderOutput OUT;
  OUT.color = IN.color;
  return OUT;
}
