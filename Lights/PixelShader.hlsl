#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
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
	
	float3 finalColor = ColorFromLight(directionalLight1, input.normal, view, colorTint, roughness) + 
						ColorFromLight(directionalLight2, input.normal, view, colorTint, roughness) +
						ColorFromLight(directionalLight3, input.normal, view, colorTint, roughness) +
						(ambientColor * colorTint);
	
	return float4(finalColor, 1);
}