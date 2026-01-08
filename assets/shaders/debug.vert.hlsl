#include "debug.h"

[[vk::push_constant]]
DebugPushConstants pushConstants;

DebugLineOutput main(uint vertId : SV_VertexID) {
  DebugLine dbgLine = vk::RawBufferLoad<DebugLine>(pushConstants.vertexBuffer + (vertId * LINESIZE));
  DebugLineOutput OUT;
  OUT.position = mul(pushConstants.viewProjection, dbgLine.position);
  OUT.color = dbgLine.color;
  return OUT;
}
