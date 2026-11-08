struct VertexInput {
  float3 pos;
  float3 normal;
  float2 texCoord;
};

struct VertexOutput {
  float4 sv_position : SV_Position;
  float3 color;
  float2 texCoord : TEXCOORD0;
};

[[vk::binding(0, 0)]]
Sampler2D albedo;

struct PushConstants {
  column_major float4x4 viewProj;
};
[[vk::push_constant]]
PushConstants pc;

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
  VertexOutput output;
  output.sv_position = mul(pc.viewProj, float4(input.pos, 1.0));
  output.color = input.normal;
  output.texCoord = input.texCoord;
  return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  return albedo.Sample(inVert.texCoord);
}
