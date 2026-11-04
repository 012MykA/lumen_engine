struct VertexInput {
  float3 pos;
  float3 color;
};

struct VertexOutput {
  float3 color;
  float4 sv_position : SV_Position;
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
  return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 color = inVert.color;
  return float4(color, 1.0);
}
