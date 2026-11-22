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

struct GlobalData {
  float4x4 view;
  float4x4 proj;
  float4x4 viewProj;
  float4 cameraPos;
  float4 time;
};

struct PushConstants {
  float4x4 model;
};

[[vk::binding(0, 0)]]
ConstantBuffer<GlobalData> gGlobal;

[[vk::binding(0, 1)]]
Sampler2D gTexture;

[[vk::push_constant]]
PushConstants gPush;

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
  VertexOutput output;
  output.sv_position =
      mul(gGlobal.viewProj, mul(gPush.model, float4(input.pos, 1.0)));
  output.color = input.normal;
  output.texCoord = input.texCoord;
  return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  return gTexture.Sample(inVert.texCoord);
}
