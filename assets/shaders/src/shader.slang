struct VertexInput {
  float3 pos;
  float3 color;
  float2 texCoord;
};

struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 color;
  float2 texCoord : TEXCOORD0;
};

struct PushConstants {
  column_major float4x4 viewProj;
};
[[vk::push_constant]]
PushConstants pc;

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
  VertexOutput output;
  output.sv_position = mul(pc.viewProj, float4(input.pos, 1.0));
  output.color = input.color;
  output.texCoord = input.texCoord;
  return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 color = inVert.color;
  return float4(color, 1.0);
}
