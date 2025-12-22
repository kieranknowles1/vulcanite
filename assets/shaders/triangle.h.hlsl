struct VertexShaderOutput {
  float4 position : SV_Position;
  float4 color : COLOR;
};

struct FragmentShaderOutput {
  float4 color : SV_Target;
};
