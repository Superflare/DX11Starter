#include "Material.h"

Material::Material(
	std::shared_ptr<SimpleVertexShader> vxShader,
	std::shared_ptr<SimplePixelShader> pxShader,
	DirectX::XMFLOAT4 colorTint,
	float roughness,
	float texScale,
	DirectX::XMFLOAT2 texOffset
	)
	:
	vertexShader(vxShader),
	pixelShader(pxShader),
	name(""),
	colorTint(colorTint),
	roughness(roughness),
	textureScale(texScale),
	textureOffset(texOffset)
{
}

void Material::Prepare()
{
	pixelShader->SetFloat4("colorTint", colorTint);
	pixelShader->SetFloat("roughness", roughness);
	pixelShader->SetFloat("uvScale", textureScale);
	pixelShader->SetFloat2("uvOffset", textureOffset);

	for (auto& s : textureSrvs)
	{
		pixelShader->SetShaderResourceView(s.first.c_str(), s.second);
	}

	for (auto& s : textureSamplers)
	{
		pixelShader->SetSamplerState(s.first.c_str(), s.second);
	}
}
