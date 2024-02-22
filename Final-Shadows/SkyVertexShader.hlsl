#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix proj;
}


VertexToPixelSky main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixelSky output;
	
	// Removes the translation portion of the camera's view matrix
	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	// Calculates position without using a world matrix
	// Skybox needs to be always centered around the main camera..
	// so we can skip accounting for any local transform changes
	output.position = mul(mul(proj, viewNoTranslation), float4(input.localPosition, 1.0f));
	output.position.z = output.position.w;
	output.sampleDir = input.localPosition;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}