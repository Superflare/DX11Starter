cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float totalTime;
}


// Struct representing the data we expect to receive from earlier pipeline stages
struct VertexToPixel
{
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;
};

// The entry point (main method) for our pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(abs(sin(totalTime * 0.8f)) * cos(input.uv.x), abs(cos(totalTime * 1.2f)) * cos(input.uv.y), sin(input.screenPosition.x + input.screenPosition.y), 1);
}