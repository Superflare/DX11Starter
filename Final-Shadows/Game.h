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
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvDragonSkin[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvRainbowDamascus[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBronze[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvCobblestone[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFloor[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvPaint[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvRough[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvScratched[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvWood[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvDefaultNormalMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> texSampler;

	// Shadow Map fields
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDsvDesc;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsvShadowMap;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> texShadowMaps;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texShadowMapArray;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvShadowMapArray;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvShadowMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowMapSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowMapRasterizer;
	int shadowMapResolution;
	int numLightsCastingShadows;
	std::vector<int> prevLightShadowSettings;
	std::vector<DirectX::XMFLOAT4X4> lightViewMatrices;
	std::vector<DirectX::XMFLOAT4X4> lightProjMatrices;

	// Game objects
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector<std::shared_ptr<Material>> materials;
	std::shared_ptr<Camera> camera;
	std::vector<Light> lights;
	std::shared_ptr<Sky> skybox;
};

