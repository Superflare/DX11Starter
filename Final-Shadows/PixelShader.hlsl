#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 6

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughnessFlat;
	float3 cameraPosition;
	Light lights[NUM_LIGHTS];
	float uvScale;
	float metallicFlat;
	float2 uvOffset;
    int skyMipCount;
    float indirectLightIntensity;
}

Texture2D Albedo					: register(t0);
Texture2D NormalMap					: register(t1);
Texture2D RoughnessMap				: register(t2);
Texture2D MetallicMap				: register(t3);
Texture2DArray<float4> ShadowMaps	: register(t4);
TextureCube SkyCubeMap				: register(t5);

SamplerState BasicSampler				: register(s0);
SamplerComparisonState ShadowSampler	: register(s1);


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
	// Calculate pixel-specific variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));

	input.uv /= uvScale;
	input.uv.x -= uvOffset.x;
	input.uv.y += uvOffset.y;
	
	float3 view = normalize(cameraPosition - input.worldPosition);
    float3 incident = -view;

	// Sample values from textures ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Get normals from a normal map texture
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	float3 bitangent = cross(input.normal, input.tangent);
	float3x3 tbn = float3x3(input.tangent, bitangent, input.normal);
	input.normal = normalize(mul(unpackedNormal, tbn));
	
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
	
	// Calculate PBR specific lighting values ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Specular color determination
	// Assume albedo texture is actually holding specular color where metalness == 1
	//
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
	float3 specularColor = lerp(F0_NON_METAL.rrr, albedo.rgb, metallic);
	
	// Calculate simple reflections for objects with the skybox and apply them with a fresnel
    float3 reflectDir = reflect(incident, input.normal);
	// Blur the reflections based on the roughness of the material
	// The blur effect is achieved cheaply by using lower mip map levels of the skybox for the reflections
	// The reflection blur ramps up quickly towards max blurriness in a quadratic rather than linear way
    float3 reflectColor = SkyCubeMap.SampleLevel(BasicSampler, reflectDir, pow(roughness, 0.3f) * (skyMipCount - 1)).rgb;
	
	// Calculate simple indirect diffuse lighting coming from the environment
    float3 indirectDiffuse = SkyCubeMap.SampleLevel(BasicSampler, input.normal, skyMipCount - 1).rgb;
    indirectDiffuse = DiffuseEnergyConserve(indirectDiffuse, specularColor, metallic);
	
	// Shadow Maps ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Sample shadow maps and save the result of the depth buffer comparison
	// Stores the unshadowed amount -- 0 is all shadow, 1 is no shadow
	// Calculate distance from each light that casts shadows
    float lightDepths[MAX_NUM_SHADOW_MAPS];
    float2 shadowUVs[MAX_NUM_SHADOW_MAPS];
    for (int i = 0; i < MAX_NUM_SHADOW_MAPS; i++)
    {
		// Because shadowPosition is not tagged as SV_POSITION we must do the perspective divide ourselves
        lightDepths[i] = input.shadowPositions[i].z / input.shadowPositions[i].w;
		// Adjust [-1 to 1] range to be [0 to 1] for Shadow Map UVs
        shadowUVs[i] = input.shadowPositions[i].xy / input.shadowPositions[i].w * 0.5f + 0.5f;
        shadowUVs[i].y = 1.0f - shadowUVs[i].y; // Flip y
    }
	
	// Get depth value of closest surface from each Shadow Map, then compare it to the actual depth of this pixel
    float shadowAmount[MAX_NUM_SHADOW_MAPS];
    for (int i = 0; i < MAX_NUM_SHADOW_MAPS; i++)
    {
		// float3(UVs to sample from, Shadow Map index in the array)
        float3 sampleAt = float3(shadowUVs[i].x, shadowUVs[i].y, i);
        shadowAmount[i] = ShadowMaps.SampleCmpLevelZero(ShadowSampler, sampleAt, lightDepths[i]);
    }
	
	
	// Calculate the final color of this pixel ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Calculate final unlit surface color
	float3 surfaceColor = albedo * colorTint.xyz;
	float3 finalColor = 0;
	// Add all light color values together
	int shadowIndex = 0;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		// Lighting calculations with shadows for point lights are different from every other type of light
		// Point lights have 6 Shadow Maps to look at while all other lights only have 1
		switch (lights[i].type)
		{
			case LIGHT_TYPE_POINT:
			{
				float3 unshadowedColor = ColorFromLight(lights[i], input.normal, input.worldPosition, view, surfaceColor, specularColor, roughness, metallic);
				float maxDot = 0.0f;
				int directionIndex = 0;
				// Find which Shadow Map to sample from by performing a dot product between the direction vector from the light to this pixel
				// and each of the 6 directions to a cube face
				// The dot product with the highest value signifies a smaller angle between vectors
				for (int j = 0; j < 6 && lights[i].castsShadows; j++)
				{
					float dotProduct = dot(normalize(input.worldPosition - lights[i].position), cubeFaceDirections[j]);
					directionIndex = dotProduct >= maxDot ? j : directionIndex;
					maxDot = max(dotProduct, maxDot);
				}
				// Use the sample only from the Shadow Map directly influencing this pixel
				finalColor += unshadowedColor * (lights[i].castsShadows ? shadowAmount[shadowIndex + directionIndex] : 1.0f);
				shadowIndex = lights[i].castsShadows ? shadowIndex + 6 : shadowIndex;
				break;
			}

			default:
			{
				// Multiply the unshadowed color by the amount of light that should be able to reach this pixel because of shadows, if this light casts shadows
				finalColor += ColorFromLight(lights[i], input.normal, input.worldPosition, view, surfaceColor, specularColor, roughness, metallic) * (lights[i].castsShadows ? shadowAmount[shadowIndex] : 1.0f);
				shadowIndex = lights[i].castsShadows ? shadowIndex + 1 : shadowIndex;
				break;
			}
		}
	}
	
    finalColor = LightenToGamma(finalColor);
	
    //float3 indirectSpecular = reflectColor * Fresnel(view, input.normal, specularColor);
    //float3 indirect = indirectSpecular + indirectDiffuse * LightenToGamma(surfaceColor.rgb);
    finalColor += indirectDiffuse * LightenToGamma(surfaceColor.rgb) * indirectLightIntensity;
	
	// Apply reflections to the edges of all objects by interpolating their shaded color with the reflection color
    finalColor = lerp(finalColor, reflectColor, Fresnel(view, input.normal, specularColor));
	
	return float4(finalColor, 1);
}