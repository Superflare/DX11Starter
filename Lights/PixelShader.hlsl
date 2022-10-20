#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	Light lights[NUM_LIGHTS];
	float3 ambientColor;
}


// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float3 view = normalize(cameraPosition - input.worldPosition);
	
	float3 finalColor = ambientColor * colorTint.xyz;

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		finalColor += ColorFromLight(lights[i], input.normal, input.worldPosition, view, colorTint.xyz, roughness);
	}
	
	return float4(finalColor, 1);
}