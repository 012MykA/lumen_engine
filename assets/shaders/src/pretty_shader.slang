/*
Shader source code at: https://shader-slang.org/

// A simple shader ported from https://www.shadertoy.com/view/XdlSDs.
// Author: dynamite
// License: CC BY-NC-SA 3.0
// https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en
*/

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
  float2 p = inVert.texCoord * 2.0 - 1.0;

  float tau = 3.14159265359 * 2.0;
  float a = atan2(p.x, p.y);
  float r = length(p) * 0.75;
  float2 uv = float2(a / tau, r);

  float t = gGlobal.time.x;

  float xCol = (uv.x - t / 3.0) * 3.0;
  xCol = fmod(abs(xCol), 3.0);
  float3 horColour = float3(0.25, 0.25, 0.25);

  if (xCol < 1.0) {
    horColour.r += 1.0 - xCol;
    horColour.g += xCol;
  } else if (xCol < 2.0) {
    xCol -= 1.0;
    horColour.g += 1.0 - xCol;
    horColour.b += xCol;
  } else {
    xCol -= 2.0;
    horColour.b += 1.0 - xCol;
    horColour.r += xCol;
  }

  uv = (2.0 * uv) - 1.0;
  float beamWidth =
      (0.7 + 0.5 * cos(uv.x * 10.0 * tau * 0.15 *
                       clamp(floor(5.0 + 10.0 * cos(t)), 0.0, 10.0))) *
      abs(1.0 / (30.0 * uv.y));
  float3 horBeam = float3(beamWidth);

  return float4(horBeam * horColour, 1.0);
}
