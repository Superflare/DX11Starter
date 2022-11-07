#pragma once

#include <d3d11.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <memory>
#include "SimpleShader.h"
#include "Mesh.h"
#include "Camera.h"

class Sky
{
public:
	Sky(
		std::shared_ptr<Mesh> mesh,
		const wchar_t* textureDdsPath,
		const wchar_t* vertexShaderPath,
		const wchar_t* pixelShaderPath,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);
	Sky(
		std::shared_ptr<Mesh> mesh,
		const wchar_t* cubeRight,
		const wchar_t* cubeLeft,
		const wchar_t* cubeUp,
		const wchar_t* cubeDown,
		const wchar_t* cubeFront,
		const wchar_t* cubeBack,
		const wchar_t* vertexShaderPath,
		const wchar_t* pixelShaderPath,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);
	~Sky();

	void Draw(std::shared_ptr<Camera> camera, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

private:
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);
	void InitResources(
		const wchar_t* vertexShaderPath,
		const wchar_t* pixelShaderPath,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);

	Microsoft::WRL::ComPtr<ID3D11SamplerState> textureSampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSrv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};

