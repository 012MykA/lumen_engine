struct VertexInput {
	[[vk::location(0)]]
	float3 position;

	[[vk::location(1)]]
	float3 normal;

	[[vk::location(2)]]
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
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
	VertexOutput output;
	output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.position, 1.0))));
	output.normal = mul((float3x3)ubo.model, input.normal);
	output.uv = input.uv;
	return output;
}

[[vk::binding(1, 0)]]
Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target {
	float4 texColor = texture.Sample(input.uv);
	float3 norm = normalize(input.normal);

	float3 lightDir = normalize(float3(0.5, 1.0, 0.5));
	float3 lightColor = float3(1.0, 1.0, 1.0);

	float diffuseFactor = max(dot(norm, lightDir), 0.0);
	float3 diffuse = diffuseFactor * lightColor;

	float3 ambient = float3(0.1);

	float3 finalColor = texColor.rgb * (diffuse + ambient);

	return float4(finalColor, texColor.a);
}
