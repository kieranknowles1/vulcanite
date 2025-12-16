[[vk::image_format("rgba16f")]]
RWTexture2D<float4> image : register(u0);

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

  float4 colour = float4(
    float(texelCoord.x) / width,
    float(texelCoord.y) / height,
    0.0f,
    1.0f
  );
  image[texelCoord] = colour;
}
