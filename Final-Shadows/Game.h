#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <vector>
#include "WICTextureLoader.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Lights.h"
#include "Sky.h"

class Game
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void LoadTextures();
	void SetupShadows(int resolution);
	void SetupLights();
	void CreateMaterials();
	void CreateEntities();

	// Update helper methods
	void UpdateUI(float dt);

	void PositionGeometry();
	void UpdateGeometry();
	void RenderShadowMaps();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> animatedPixelShader;
	std::shared_ptr<SimpleVertexShader> shadowMapVertexShader;

	// Textures, SRVs, and Sampler States
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSnowglobe[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvChristmasTree;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSnowman;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvDefaultNormalMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> texSampler;

	// Shadow Map fields
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsvShadowMap;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> texShadowMaps;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texShadowMapArray;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvShadowMapArray;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowMapSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowMapRasterizer;

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDsvDesc;
	D3D11_TEXTURE2D_DESC shadowMapTextureArrayDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc;

	std::vector<DirectX::XMFLOAT4X4> lightViewMatrices;
	std::vector<DirectX::XMFLOAT4X4> lightProjMatrices;
	std::vector<int> prevLightShadowSettings;
	const std::vector<DirectX::XMFLOAT3> cubeFaceDirections = 
	{
		DirectX::XMFLOAT3(0.f, 0.f, 1.f),
		DirectX::XMFLOAT3(1.f, 0.f, 0.f),
		DirectX::XMFLOAT3(0.f, 0.f, -1.f),
		DirectX::XMFLOAT3(-1.f, 0.f, 0.f),
		DirectX::XMFLOAT3(0.f, 1.f, 0.f),
		DirectX::XMFLOAT3(0.f, -1.f, 0.f)
	};

	int shadowMapResolution;
	int numShadowMaps;


	// Game objects
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector<std::shared_ptr<Material>> materials;
	std::shared_ptr<Camera> camera;
	std::vector<Light> lights;
	std::shared_ptr<Sky> skybox;
};

