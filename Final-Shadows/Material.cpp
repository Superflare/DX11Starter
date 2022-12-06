#include "Material.h"

Material::Material(
	const char* name,
	std::shared_ptr<SimpleVertexShader> vxShader,
	std::shared_ptr<SimplePixelShader> pxShader,
	DirectX::XMFLOAT4 colorTint,
	float roughness,
	float metallic,
	float texScale,
	DirectX::XMFLOAT2 texOffset
	)
	:
	name(name),
	vertexShader(vxShader),
	pixelShader(pxShader),
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

void Material::SetAllPbrTextures(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textures[4])
{
	if (textures[0].Get() != nullptr)
	{
		SetAlbedo(textures[0]);
	}

	if (textures[1].Get() != nullptr)
	{
		SetNormal(textures[1]);
	}

	if (textures[2].Get() != nullptr)
	{
		SetRoughness(textures[2]);
	}

	if (textures[3].Get() != nullptr)
	{
		SetMetallic(textures[3]);
	}
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
