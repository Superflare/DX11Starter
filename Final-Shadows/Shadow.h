#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

#include "Lights.h"
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"

class Shadow
{
public:
	Shadow();
	Shadow(int cascadeHighestRes, int worldMapRes, int numCascades,
		const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context);
	// Copy Constructor
	Shadow(Shadow& other) = delete;
	// Copy Assignment
	Shadow& operator=(Shadow& other) = delete;
	// Move Constructor
	//Shadow(Shadow&& other);
	// Move Assignment
	Shadow& operator=(Shadow&& other);
	~Shadow();

	void Init(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context);
	void SetupTextures(const std::vector<Light>& lights, const Microsoft::WRL::ComPtr<ID3D11Device>& device);
	void Update(const std::vector<Light>& lights, const Microsoft::WRL::ComPtr<ID3D11Device>& device);
	void UpdateTexNumber(int numCascades, int numWorldPosMaps, const Microsoft::WRL::ComPtr<ID3D11Device>& device);
	void Resize(int cascadeHighestRes, int worldMapRes, const Microsoft::WRL::ComPtr<ID3D11Device>& device);

	void Render
	(
		const Microsoft::WRL::ComPtr<ID3D11Device>& device,
		const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
		const std::vector<Light>& lights,
		const std::vector<std::shared_ptr<GameEntity>>& entities,
		const std::shared_ptr<Camera>& cam
	);
	void InternalRenderCascade
	(
		const Microsoft::WRL::ComPtr<ID3D11Device>& device,
		const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
		const std::vector<Light>& lights,
		const std::vector<std::shared_ptr<GameEntity>>& entities,
		const std::shared_ptr<Camera>& cam
	);
	void InternalRenderWorldPos
	(
		const Microsoft::WRL::ComPtr<ID3D11Device>& device,
		const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
		const std::vector<Light>& lights,
		const std::vector<std::shared_ptr<GameEntity>>& entities
	);
	bool CreateLightMatrices
	(
		const Light& l,
		std::vector<DirectX::XMFLOAT4X4>& view,
		std::vector<DirectX::XMFLOAT4X4>& proj,
		const std::shared_ptr<Camera>& cam,
		int idx = 0
	);

	inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetDirLightShadowSrv() { return m_CascadeSrv; }
	inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetWorldPosLightShadowSrv() { return m_WorldPosSrv; }
	inline std::vector<DirectX::XMFLOAT4X4> GetDirLightViews() { return m_CascadeLightViews; }
	inline std::vector<DirectX::XMFLOAT4X4> GetDirLightProjs() { return m_CascadeLightProjs; }
	inline std::vector<DirectX::XMFLOAT4X4> GetWorldPosLightViews() { return m_WorldLightViews; }
	inline std::vector<DirectX::XMFLOAT4X4> GetWorldPosLightProjs() { return m_WorldLightProjs; }
	inline Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSampler() { return m_Sampler; }
	inline const int GetNumWorldPosShadowMaps() { return m_NumWorldPosMaps; }
	inline const int GetNumCascadeShadowMaps() { return m_NumCascades; }

private:
	int m_CascadeHighestRes;
	int m_WorldMapRes;

	int m_NumCascades;
	int m_NumWorldPosMaps;

	std::vector<int> m_lightSettings;

	std::shared_ptr<SimpleVertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_Dsv;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_DepthTexturesCascade;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_DepthTexturesWorld;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_TexArrayCascade;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_TexArrayWorld;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_CascadeSrv;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_WorldPosSrv;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_Sampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_Rasterizer;
	D3D11_TEXTURE2D_DESC m_TexDescCascade;
	D3D11_TEXTURE2D_DESC m_TexArrayDescCascade;
	D3D11_SHADER_RESOURCE_VIEW_DESC m_SrvDescCascade;
	D3D11_TEXTURE2D_DESC m_TexDescWorld;
	D3D11_TEXTURE2D_DESC m_TexArrayDescWorld;
	D3D11_SHADER_RESOURCE_VIEW_DESC m_SrvDescWorld;
	D3D11_DEPTH_STENCIL_VIEW_DESC m_DsvDesc;

	std::vector<DirectX::XMFLOAT4X4> m_CascadeLightViews;
	std::vector<DirectX::XMFLOAT4X4> m_CascadeLightProjs;
	std::vector<DirectX::XMFLOAT4X4> m_WorldLightViews;
	std::vector<DirectX::XMFLOAT4X4> m_WorldLightProjs;

	const DirectX::XMFLOAT3 cubeFaceDirections[6] =
	{
		DirectX::XMFLOAT3(0.f, 0.f, 1.f),
		DirectX::XMFLOAT3(1.f, 0.f, 0.f),
		DirectX::XMFLOAT3(0.f, 0.f, -1.f),
		DirectX::XMFLOAT3(-1.f, 0.f, 0.f),
		DirectX::XMFLOAT3(0.f, 1.f, 0.f),
		DirectX::XMFLOAT3(0.f, -1.f, 0.f)
	};
};

