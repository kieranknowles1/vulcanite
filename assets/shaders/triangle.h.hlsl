struct VertexShaderOutput {
  float4 position : SV_Position;
  float4 color : COLOR;
  float2 uv : TEXCOORD0;
};

struct FragmentShaderOutput {
  float4 color : SV_Target;
};
