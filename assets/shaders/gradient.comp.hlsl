#include "interop.h"

[[vk::image_format("rgba16f")]]
RWTexture2D<float4> image : register(u0);

[[vk::push_constant]]
GradientPushConstants pushConstants;

// Very simple test shader that fills a texture with a gradient
[numthreads(16, 16, 1)]
void main(
  uint3 dispatchThreadId : SV_DispatchThreadID,
  uint3 groupThreadId : SV_GroupThreadID
) {
  uint2 texelCoord = dispatchThreadId.xy;

  uint width; uint height;
  image.GetDimensions(width, height);
  if (texelCoord.x >= width || texelCoord.y >= height)
    return;
  if (groupThreadId.x == 0 || groupThreadId.y == 0)
    return;

  float2 blendFactor = float2(
    float(texelCoord.x) / width,
    float(texelCoord.y) / height
  );

  float4 tlc = pushConstants.topLeftColor;
  float4 brc = pushConstants.bottomRightColor;

  float4 colour = float4(
    lerp(tlc.x, brc.x, blendFactor.x),
    lerp(tlc.y, brc.y, blendFactor.y),
    lerp(tlc.z, brc.z, blendFactor.x),
    lerp(tlc.w, brc.w, blendFactor.y)
  );

  image[texelCoord] = colour;
}
