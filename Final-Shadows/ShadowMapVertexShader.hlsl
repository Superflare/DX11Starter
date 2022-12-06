#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
}

float4 main( VertexShaderInput input ) : SV_POSITION
{
	matrix wvp = mul(proj, mul(view, world));
	return mul(wvp, float4(input.localPosition, 1.0f));
}