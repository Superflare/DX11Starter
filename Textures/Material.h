#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include "SimpleShader.h"

class Material
{
public:
	Material(std::shared_ptr<SimpleVertexShader> vxShader, std::shared_ptr<SimplePixelShader> pxShader,
		DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(1,1,1,1), float roughness = 0.0f);

	std::shared_ptr<SimpleVertexShader> GetVertexShader() { return vertexShader; }
	std::shared_ptr<SimplePixelShader> GetPixelShader() { return pixelShader; }
	DirectX::XMFLOAT4 GetColorTint() { return colorTint; }
	float GetRoughness() { return roughness; }

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vxShader) { vertexShader = vxShader; }
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pxShader) { pixelShader = pxShader; }
	void SetColorTint(DirectX::XMFLOAT4 color) { colorTint = color; }
	void SetRoughness(float val) { roughness = val; }

	void AddTextureSrv(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) { textureSrvs.insert({shaderName, srv}); }
	void AddSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler) { textureSamplers.insert({shaderName, sampler}); }

	void Prepare();

private:
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	
	DirectX::XMFLOAT4 colorTint;
	float roughness;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSrvs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> textureSamplers;
};

