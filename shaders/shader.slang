struct VertexInput {
	float2 pos : POSITION;
	float3 color : COLOR;
};
struct UniformBuffer {
	float4x4 model;
	float4x4 view;
	float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput {
	float4 sv_position : SV_Position;
	float3 color : COLOR;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
	VertexOutput output;
	output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.proj, float4(input.pos, 0.0, 1.0))));
	output.color = input.color;
	return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target {
	return float4(input.color, 1.0);
}
