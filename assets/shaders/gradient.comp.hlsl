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

  float2 blendFactor = float2(
    float(texelCoord.x) / width,
    float(texelCoord.y) / height
  );

  float4 lc = pushConstants.leftColor;
  float4 rc = pushConstants.rightColor;

  float4 color = lerp(lc, rc, blendFactor.x);

  image[texelCoord] = color;
}
