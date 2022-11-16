#include "Material.h"

Material::Material(
	std::shared_ptr<SimpleVertexShader> vxShader,
	std::shared_ptr<SimplePixelShader> pxShader,
	DirectX::XMFLOAT4 colorTint,
	float roughness,
	float metallic,
	float texScale,
	DirectX::XMFLOAT2 texOffset
	)
	:
	vertexShader(vxShader),
	pixelShader(pxShader),
	name(""),
	colorTint(colorTint),
	roughness(roughness),
	metallic(metallic),
	textureScale(texScale),
	textureOffset(texOffset)
{
}

void Material::SetRoughness(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSrvs.insert_or_assign("RoughnessMap", srv);
	roughness = -1;
}

void Material::SetMetallic(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSrvs.insert_or_assign("MetallicMap", srv);
	metallic = -1;
}

void Material::Prepare()
{
	pixelShader->SetFloat4("colorTint", colorTint);
	pixelShader->SetFloat("roughnessFlat", roughness);
	pixelShader->SetFloat("metallicFlat", metallic);
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
