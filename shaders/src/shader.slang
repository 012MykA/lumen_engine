struct VertexInput {
	float3 position;
	float3 normal;
	float2 uv;
};

struct VertexOutput {
	float4 sv_position : SV_Position;
	float3 normal;
	float2 uv;
};

struct UniformBuffer {
	float4x4 model;
	float4x4 view;
	float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
	VertexOutput output;
	output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.position, 1.0))));
	output.normal = input.normal;
	output.uv = input.uv;
	return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target {
	float4 texColor = texture.Sample(input.uv);
	return texColor;
}
