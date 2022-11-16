#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughnessFlat;
	float3 cameraPosition;
	Light lights[NUM_LIGHTS];
	float uvScale;
	float metallicFlat;
	float2 uvOffset;
}

Texture2D Albedo		  : register(t0);
Texture2D NormalMap		  : register(t1);
Texture2D RoughnessMap	  : register(t2);
Texture2D MetallicMap	  : register(t3);
SamplerState BasicSampler : register(s0);


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
	// Calculate pixel-specific variables
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));
	input.uv /= uvScale;
	input.uv.x -= uvOffset.x;
	input.uv.y += uvOffset.y;
	float3 view = normalize(cameraPosition - input.worldPosition);


	// Get normals from a normal map texture
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	float3 bitangent = cross(input.tangent, input.normal);
	float3x3 tbn = float3x3(input.tangent, bitangent, input.normal);
	input.normal = normalize(mul(unpackedNormal, tbn));
	
	// Sample values from textures
	// Get base surface color
	float3 albedo = DarkenToGamma(Albedo.Sample(BasicSampler, input.uv).rgb);
	// Get roughness value from texture, if a texture is provided
	float roughness = roughnessFlat;
	if (roughnessFlat == -1)
		roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
	roughness = max(roughness, MIN_ROUGHNESS);
	// Get metallic value from texture, if a texture is provided
	float metallic = metallicFlat;
	if (metallicFlat == -1)
		metallic = MetallicMap.Sample(BasicSampler, input.uv).r;

	// Specular color determination -----------------
	// Assume albedo texture is actually holding specular color where metalness == 1
	//
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
	float3 specularColor = lerp(F0_NON_METAL.rrr, albedo.rgb, metallic);
	
	// Calculate the final color of this pixel
	// Calculate final unlit surface color
	float3 surfaceColor = albedo * colorTint.xyz;
	float3 finalColor = 0;
	// Add all light color values together
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		finalColor += ColorFromLight(lights[i], input.normal, input.worldPosition, view,  surfaceColor, specularColor, roughness, metallic);
	}
	
	return float4(LightenToGamma(finalColor), 1);
}