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
#include "Shadow.h"

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
	void SetupLights();
	void CreateMaterials();
	void CreateEntities();

	// Update helper methods
	void UpdateUI(float dt);

	void PositionGeometry();
	void UpdateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> animatedPixelShader;

	// Textures, SRVs, and Sampler States
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvDefaultGrid;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSnowglobe[4];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvChristmasTree;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSnowman;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvDefaultNormalMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> texSampler;

	// Shadow handler
	Shadow shadow;

	// UI Options
	float indirectLightIntensity;

	// Game objects
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector<std::shared_ptr<Material>> materials;
	std::shared_ptr<Camera> camera;
	std::vector<Light> lights;
	std::shared_ptr<Sky> skybox;
};

