#include "debug.h"

FragmentShaderOutput main(DebugLineOutput IN) {
  FragmentShaderOutput OUT;
  OUT.color = IN.color;
  return OUT;
}
