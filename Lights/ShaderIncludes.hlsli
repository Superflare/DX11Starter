#ifndef SHADER_INCLUDES
#define SHADER_INCLUDES

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2
#define MAX_SPECULAR_EXPONENT	256.0f

struct Light {
	int type;						// directional, point, spot
	float3 direction;
	float range;					// attenuation range
	float3 position;
	float intensity;
	float3 color;
	float spotFalloff;				// spot light cone size
	float3 padding;					// purposeful padding to hit the 16-byte boundary
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 worldPosition	: POSITION;
};

float3 DirToLight(Light light) { return normalize(-light.direction); }
float3 Diffuse(float3 dirToLight, float3 normal) { return saturate(dot(normal, dirToLight)); }
float3 Specular(Light light, float3 normal, float3 view, float exponent)
{
	if (exponent > 0.05f)
	{
		float3 r = reflect(normalize(light.direction), normal);
		return pow(saturate(dot(r, view)), exponent);
	}

	return float3(0, 0, 0);
}
float3 ColorFromLight(Light light, float3 normal, float3 view, float3 surfaceColor, float roughness)
{
	float3 dirToLight = DirToLight(light);

	float3 diffuse = Diffuse(dirToLight, normal);

	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 specular = Specular( light, normal, view, specExponent);

	return (diffuse * light.color * surfaceColor) + (specular * light.color);
}

#endif