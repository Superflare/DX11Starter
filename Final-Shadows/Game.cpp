#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "ImGuiMenus.h"
#include "Material.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	// Initialize the game's camera
	XMFLOAT4 camStartRot;
	XMStoreFloat4(&camStartRot, XMQuaternionRotationAxis(XMVectorSet(1,0,0,0), Deg2Rad(30)));
	camera = std::make_shared<Camera>(XMFLOAT3(-2.0f, 10.0f, -15.0f), camStartRot, (float)1280 / 720);
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for each init task
	LoadShaders();
	CreateGeometry();
	LoadTextures();

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	// Define and create the Sampler State
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, texSampler.GetAddressOf());

	CreateMaterials();
	CreateEntities();
	SetupLights();
	SetupShadows(1024);

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str());

	pixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str());

	animatedPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"AnimatedPixelShader.cso").c_str());

	shadowMapVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"ShadowMapVertexShader.cso").c_str());
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context));
}

// Create a list of Game Entities to be rendered to the screen and initialize their starting transforms
// Not every mesh used in the game entities is centered at the origin so transformations are relative
void Game::CreateEntities()
{
	// Set up the Game Entity list using the pre-created meshes
	// PBR comparison entities
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[3]));
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[4]));
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[5]));
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[7]));
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[2]));
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[6]));
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[8]));
	// Cube
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[1]));
	// Cylinder
	entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[0]));
	// Helix
	entities.push_back(std::make_shared<GameEntity>(meshes[2], materials[1]));
	// Quad
	entities.push_back(std::make_shared<GameEntity>(meshes[3], materials[1]));
	// Quad Double Sided
	entities.push_back(std::make_shared<GameEntity>(meshes[4], materials[0]));
	// Sphere
	entities.push_back(std::make_shared<GameEntity>(meshes[5], materials[0]));
	// Torus
	entities.push_back(std::make_shared<GameEntity>(meshes[6], materials[1]));

	// Floor
	entities.push_back(std::make_shared<GameEntity>(meshes[3], materials[8]));

	PositionGeometry();
}

void Game::CreateMaterials()
{
	// Rainbow Damascus
	std::shared_ptr<Material> mRainbowDamascus = std::make_shared<Material>("Rainbow Damascus", vertexShader, pixelShader);
	mRainbowDamascus->SetAllPbrTextures(srvRainbowDamascus);
	mRainbowDamascus->AddSampler("BasicSampler", texSampler);
	// Dragon Skin
	std::shared_ptr<Material> mDragonSkin = std::make_shared<Material>("Dragon Skin", vertexShader, pixelShader);
	mDragonSkin->SetAllPbrTextures(srvDragonSkin);
	mDragonSkin->AddSampler("BasicSampler", texSampler);
	// Bronze
	std::shared_ptr<Material> mBronze = std::make_shared<Material>("Bronze", vertexShader, pixelShader);
	mBronze->SetAllPbrTextures(srvBronze);
	mBronze->AddSampler("BasicSampler", texSampler);
	// Cobblestone
	std::shared_ptr<Material> mCobblestone = std::make_shared<Material>("Cobblestone", vertexShader, pixelShader);
	mCobblestone->SetAllPbrTextures(srvCobblestone);
	mCobblestone->AddSampler("BasicSampler", texSampler);
	// Floor
	std::shared_ptr<Material> mFloor = std::make_shared<Material>("Floor", vertexShader, pixelShader);
	mFloor->SetAllPbrTextures(srvFloor);
	mFloor->AddSampler("BasicSampler", texSampler);
	// Paint
	std::shared_ptr<Material> mPaint = std::make_shared<Material>("Paint", vertexShader, pixelShader);
	mPaint->SetAllPbrTextures(srvPaint);
	mPaint->AddSampler("BasicSampler", texSampler);
	// Rough
	std::shared_ptr<Material> mRough = std::make_shared<Material>("Rough", vertexShader, pixelShader);
	mRough->SetAllPbrTextures(srvRough);
	mRough->AddSampler("BasicSampler", texSampler);
	// Scratched
	std::shared_ptr<Material> mScratched = std::make_shared<Material>("Scratched", vertexShader, pixelShader);
	mScratched->SetAllPbrTextures(srvScratched);
	mScratched->AddSampler("BasicSampler", texSampler);
	// Wood
	std::shared_ptr<Material> mWood = std::make_shared<Material>("Wood", vertexShader, pixelShader);
	mWood->SetAllPbrTextures(srvWood);
	mWood->AddSampler("BasicSampler", texSampler);

	// Custom material with animated pixel shader
	//std::shared_ptr<Material> mAnimated = std::make_shared<Material>("Animated", vertexShader, animatedPixelShader);

	materials.push_back(mRainbowDamascus);
	materials.push_back(mDragonSkin);
	materials.push_back(mBronze);
	materials.push_back(mCobblestone);
	materials.push_back(mFloor);
	materials.push_back(mPaint);
	materials.push_back(mRough);
	materials.push_back(mScratched);
	materials.push_back(mWood);
}

void Game::SetupLights()
{
	// Directional lights
	Light ldTopRight = {};
	ldTopRight.type = LIGHT_TYPE_DIRECTIONAL;
	ldTopRight.direction = XMFLOAT3(-0.3f, -0.9f, 0.f);
	ldTopRight.color = XMFLOAT3(0.9f, 0.4f, 0.1f);
	ldTopRight.intensity = 2.68f;
	ldTopRight.castsShadows = 1;

	Light ldFrontLeft = {};
	ldFrontLeft.type = LIGHT_TYPE_DIRECTIONAL;
	ldFrontLeft.direction = XMFLOAT3(-0.3f, 0, 1);
	ldFrontLeft.color = XMFLOAT3(0.76f, 0, 0.97f);
	ldFrontLeft.intensity = 0.78f;

	Light ldBackRight = {};
	ldBackRight.type = LIGHT_TYPE_DIRECTIONAL;
	ldBackRight.direction = XMFLOAT3(1, 0, -0.5f);
	ldBackRight.color = XMFLOAT3(0.12f, 0.55f, 0.84f);
	ldBackRight.intensity = 2.7f;

	// Point lights
	Light lpLeftAbove = {};
	lpLeftAbove.type = LIGHT_TYPE_POINT;
	lpLeftAbove.position = XMFLOAT3(-6.0f, 0.5f, -1.5f);
	lpLeftAbove.color = XMFLOAT3(0.12f, 0.55f, 0.84f);
	lpLeftAbove.intensity = 4.7f;
	lpLeftAbove.range = 10.0f;

	Light lpRightAbove = {};
	lpRightAbove.type = LIGHT_TYPE_POINT;
	lpRightAbove.position = XMFLOAT3(6.0f, 3.0f, 0);
	lpRightAbove.color = XMFLOAT3(1, 1, 1);
	lpRightAbove.intensity = 2.0f;
	lpRightAbove.range = 7.0f;

	Light lSpotlight = {};
	lSpotlight.type = LIGHT_TYPE_SPOT;
	lSpotlight.direction = XMFLOAT3(0.f, -1.f, 0.f);
	lSpotlight.position = XMFLOAT3(0.f, 6.f, 0.f);
	lSpotlight.color = XMFLOAT3(1.f, 1.f, 1.f);
	lSpotlight.intensity = 3.f;
	lSpotlight.range = 10.f;
	lSpotlight.spotFalloff = Deg2Rad(90.f);
	lSpotlight.castsShadows = 1;

	lights.push_back(ldTopRight);
	//lights.push_back(ldFrontLeft);
	//lights.push_back(ldBackRight);
	//lights.push_back(lpLeftAbove);
	//lights.push_back(lpRightAbove);
	lights.push_back(lSpotlight);
}

// Creates reusable descriptions for Shadow Map resources
void Game::SetupShadows(int resolution)
{
	shadowMapDsvDesc = {};
	shadowMapDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowMapDsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowMapDsvDesc.Texture2D.MipSlice = 0;

	shadowMapResolution = resolution;
	numLightsCastingShadows = 0;
	prevLightShadowSettings.clear();
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i].castsShadows == 1)
			numLightsCastingShadows++;

		prevLightShadowSettings.push_back(lights[i].castsShadows);
	}

	D3D11_TEXTURE2D_DESC shadowMapTextureDesc = {};
	shadowMapTextureDesc.Width = shadowMapResolution;
	shadowMapTextureDesc.Height = shadowMapResolution;
	shadowMapTextureDesc.MipLevels = 1;
	shadowMapTextureDesc.ArraySize = 1;
	shadowMapTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapTextureDesc.SampleDesc.Count = 1;
	shadowMapTextureDesc.SampleDesc.Quality = 0;
	shadowMapTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapTextureDesc.CPUAccessFlags = 0;
	shadowMapTextureDesc.MiscFlags = 0;

	D3D11_TEXTURE2D_DESC shadowMapTextureArrayDesc = {};
	shadowMapTextureArrayDesc.Width = shadowMapResolution;
	shadowMapTextureArrayDesc.Height = shadowMapResolution;
	shadowMapTextureArrayDesc.MipLevels = 1;
	shadowMapTextureArrayDesc.ArraySize = numLightsCastingShadows;
	shadowMapTextureArrayDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapTextureArrayDesc.SampleDesc.Count = 1;
	shadowMapTextureArrayDesc.SampleDesc.Quality = 0;
	shadowMapTextureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapTextureArrayDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapTextureArrayDesc.CPUAccessFlags = 0;
	shadowMapTextureArrayDesc.MiscFlags = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {};
	shadowMapSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowMapSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	//shadowMapSrvDesc.Texture2D.MipLevels = 1;
	//shadowMapSrvDesc.Texture2D.MostDetailedMip = 0;
	shadowMapSrvDesc.Texture2DArray.ArraySize = numLightsCastingShadows;
	shadowMapSrvDesc.Texture2DArray.MipLevels = 1;
	shadowMapSrvDesc.Texture2DArray.MostDetailedMip = 0;
	shadowMapSrvDesc.Texture2DArray.FirstArraySlice = 0;

	D3D11_SAMPLER_DESC shadowSamplerDesc = {};
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSamplerDesc, shadowMapSampler.GetAddressOf());

	D3D11_RASTERIZER_DESC shadowRasterizerDesc = {};
	shadowRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	shadowRasterizerDesc.CullMode = D3D11_CULL_BACK;
	shadowRasterizerDesc.DepthClipEnable = true;
	shadowRasterizerDesc.DepthBias = 1000;
	shadowRasterizerDesc.DepthBiasClamp = 0.0f;
	shadowRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRasterizerDesc, shadowMapRasterizer.GetAddressOf());

	if (texShadowMaps.size() > 0)
		texShadowMaps.clear();

	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i].castsShadows == 1)
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texShadowMap;
			device->CreateTexture2D(&shadowMapTextureDesc, 0, texShadowMap.GetAddressOf());

			if (texShadowMap != 0)
			{
				texShadowMaps.push_back(texShadowMap);
			}
		}
	}

	device->CreateTexture2D(&shadowMapTextureArrayDesc, 0, texShadowMapArray.ReleaseAndGetAddressOf());
	for (int i = 0; i < numLightsCastingShadows; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(0, i, 1);

		// Copy from the individual Shadow Maps to the Shadow Map Array
		context->CopySubresourceRegion(
			texShadowMapArray.Get(),
			subresource,           
			0, 0, 0,               
			texShadowMaps[i].Get(),
			0,                     
			0
		);
	}
	if (numLightsCastingShadows > 0)
		device->CreateShaderResourceView(texShadowMapArray.Get(), &shadowMapSrvDesc, srvShadowMapArray.GetAddressOf());
	//if (texShadowMaps.size() > 0)
	//	device->CreateShaderResourceView(texShadowMaps[0].Get(), &shadowMapSrvDesc, srvShadowMap.ReleaseAndGetAddressOf());
}


void Game::LoadTextures()
{
	// Bronze
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str(),
		nullptr,
		srvBronze[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/bronze_normal.png").c_str(),
		nullptr,
		srvBronze[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str(),
		nullptr,
		srvBronze[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/bronze_metallic.png").c_str(),
		nullptr,
		srvBronze[3].GetAddressOf()
	);

	// Cobblestone
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_albedo.png").c_str(),
		nullptr,
		srvCobblestone[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_normal.png").c_str(),
		nullptr,
		srvCobblestone[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_roughness.png").c_str(),
		nullptr,
		srvCobblestone[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_metallic.png").c_str(),
		nullptr,
		srvCobblestone[3].GetAddressOf()
	);

	// Floor
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/floor_albedo.png").c_str(),
		nullptr,
		srvFloor[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/floor_normal.png").c_str(),
		nullptr,
		srvFloor[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/floor_roughness.png").c_str(),
		nullptr,
		srvFloor[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/floor_metallic.png").c_str(),
		nullptr,
		srvFloor[3].GetAddressOf()
	);

	// Paint
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/paint_albedo.png").c_str(),
		nullptr,
		srvPaint[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/paint_normal.png").c_str(),
		nullptr,
		srvPaint[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/paint_roughness.png").c_str(),
		nullptr,
		srvPaint[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/paint_metallic.png").c_str(),
		nullptr,
		srvPaint[3].GetAddressOf()
	);

	// Rough
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rough_albedo.png").c_str(),
		nullptr,
		srvRough[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rough_normal.png").c_str(),
		nullptr,
		srvRough[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rough_roughness.png").c_str(),
		nullptr,
		srvRough[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rough_metallic.png").c_str(),
		nullptr,
		srvRough[3].GetAddressOf()
	);

	// Scratched
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str(),
		nullptr,
		srvScratched[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/scratched_normal.png").c_str(),
		nullptr,
		srvScratched[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str(),
		nullptr,
		srvScratched[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/scratched_metallic.png").c_str(),
		nullptr,
		srvScratched[3].GetAddressOf()
	);

	// Wood
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/wood_albedo.png").c_str(),
		nullptr,
		srvWood[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/wood_normal.png").c_str(),
		nullptr,
		srvWood[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/wood_roughness.png").c_str(),
		nullptr,
		srvWood[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/wood_metallic.png").c_str(),
		nullptr,
		srvWood[3].GetAddressOf()
	);

	// Dragon Skin
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/dragonskin_albedo.png").c_str(),
		nullptr,
		srvDragonSkin[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/dragonskin_normal.png").c_str(),
		nullptr,
		srvDragonSkin[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/dragonskin_roughness.png").c_str(),
		nullptr,
		srvDragonSkin[2].GetAddressOf()
	);

	// Rainbow Damascus
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rainbowdamascus_albedo.png").c_str(),
		nullptr,
		srvRainbowDamascus[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rainbowdamascus_normal.png").c_str(),
		nullptr,
		srvRainbowDamascus[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rainbowdamascus_roughness.png").c_str(),
		nullptr,
		srvRainbowDamascus[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/rainbowdamascus_metallic.png").c_str(),
		nullptr,
		srvRainbowDamascus[3].GetAddressOf()
	);

	// Default normal map
	/*CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/flat_normals.png").c_str(),
		nullptr,
		srvDefaultNormalMap.GetAddressOf()
	);*/

	// Create Skybox
	skybox = std::make_shared<Sky>(
		meshes[0],
		FixPath(L"../../Assets/Textures/right.png").c_str(),
		FixPath(L"../../Assets/Textures/left.png").c_str(),
		FixPath(L"../../Assets/Textures/up.png").c_str(),
		FixPath(L"../../Assets/Textures/down.png").c_str(),
		FixPath(L"../../Assets/Textures/front.png").c_str(),
		FixPath(L"../../Assets/Textures/back.png").c_str(),
		FixPath(L"SkyVertexShader.cso").c_str(),
		FixPath(L"SkyPixelShader.cso").c_str(),
		texSampler,
		device,
		context
		);
}

void Game::UpdateUI(float dt)
{
	// Get a reference to our custom input manager
	Input& input = Input::GetInstance();

	// Reset input manager's gui state so we don't
	input.SetKeyboardCapture(false);
	input.SetMouseCapture(false);

	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = dt;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;
	io.KeyCtrl = input.KeyDown(VK_CONTROL);
	io.KeyShift = input.KeyDown(VK_SHIFT);
	io.KeyAlt = input.KeyDown(VK_MENU);
	io.MousePos.x = (float)input.GetMouseX();
	io.MousePos.y = (float)input.GetMouseY();
	io.MouseDown[0] = input.MouseLeftDown();
	io.MouseDown[1] = input.MouseRightDown();
	io.MouseDown[2] = input.MouseMiddleDown();
	io.MouseWheel = input.GetMouseWheel();
	input.GetKeyArray(io.KeysDown, 256);

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);
}

void Game::PositionGeometry()
{
	// Layout each entity in a horizontal line
	for (int i = 0; i < entities.size(); i++)
	{
		float lerpBegin = -10.0f;
		float lerpEnd = 10.0f;
		float lerpPos = lerpBegin + ((i % 7) / ((float)entities.size() / 2.0f)) * (lerpEnd - lerpBegin);
		entities[i]->GetTransform()->SetPosition(lerpPos, 0, 0);
	}

	// Move my custom game entities behind the starting camera
	for (int i = 7; i < 14; i++)
	{
		entities[i]->GetTransform()->MoveAbsolute(0, 0, -20);
	}

	// Create a large floor out of a plane
	std::shared_ptr<GameEntity> floor = entities[entities.size() - 1];
	floor->GetTransform()->MoveRelative(10.f, -1.f, 0.f);
	floor->GetTransform()->Scale(20.f, 1.f, 20.f);
}

void Game::UpdateGeometry()
{
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	if (camera != 0)
	{
		camera->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	UpdateUI(deltaTime);
	ImGuiMenus::WindowStats(windowWidth, windowHeight);
	ImGuiMenus::EditScene(camera, entities, materials, &lights);

	// Update the camera
	if (camera != 0)
	{
		camera->Update(deltaTime);
	}
	
	UpdateGeometry();

	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i].castsShadows != prevLightShadowSettings[i])
		{
			SetupShadows(shadowMapResolution);
			break;
		}
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	RenderShadowMaps();

	// Render all objects in the scene
	for (int i = 0; i < entities.size(); i++)
	{
		// Animated Pixel Shader needs the totalTime var
		std::shared_ptr<SimplePixelShader> ps = entities[i]->GetMaterial()->GetPixelShader();
		ps->SetFloat("totalTime", totalTime);
		ps->SetData("lights", &lights[0], (int)lights.size() * sizeof(Light));
		context->PSSetShaderResources(5, (unsigned int)numLightsCastingShadows, srvShadowMapArray.GetAddressOf());
		//ps->SetShaderResourceView("ShadowMaps", srvShadowMapArray);
		ps->SetSamplerState("ShadowSampler", shadowMapSampler);
		std::shared_ptr<SimpleVertexShader> vs = entities[i]->GetMaterial()->GetVertexShader();
		if (lightViewMatrices.size() > 0)
		{
			vs->SetData("lightViews", &lightViewMatrices[0], numLightsCastingShadows * sizeof(XMFLOAT4X4));
			vs->SetData("lightProjs", &lightProjMatrices[0], numLightsCastingShadows * sizeof(XMFLOAT4X4));
		}

		entities[i]->Draw(context, camera);
	}

	// Draw the Skybox after each entity in the scene so that only the visible parts of the Skybox are rendered
	skybox->Draw(camera, context);

	// Draw ImGui UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		swapChain->Present(vsync ? 1 : 0, 0);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}

void Game::RenderShadowMaps()
{
	lightViewMatrices.clear();
	lightProjMatrices.clear();

	// Render scene from the pov of each light that casts shadows, and store the depth buffer as a shadow map
	int shadowIndex = 0;
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i].castsShadows == 1)
		{
			// Clear the shadow map depth buffer
			if (dsvShadowMap != 0)
				context->ClearDepthStencilView(dsvShadowMap.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
			device->CreateDepthStencilView(texShadowMaps[shadowIndex].Get(), &shadowMapDsvDesc, dsvShadowMap.ReleaseAndGetAddressOf());

			XMFLOAT4X4 lightView;
			XMFLOAT4X4 lightProj;

			switch (lights[i].type)
			{
			case LIGHT_TYPE_DIRECTIONAL:
			{
				XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&lights[i].direction));
				XMVECTOR position = -20 * lightDir;

				XMVECTOR forward = XMVectorSet(0.f, 0.f, 1.f, 1.f);
				float dot;
				XMStoreFloat(&dot, XMVector3Dot(forward, lightDir));
				float angle = acos(dot);
				XMVECTOR axis = XMVector3Cross(forward, lightDir);

				Transform lightTransform = Transform();
				lightTransform.SetRotation(XMQuaternionRotationAxis(axis, angle));
				XMFLOAT3 lookDir = lightTransform.GetForward();
				XMFLOAT3 upDir = lightTransform.GetUp();

				XMStoreFloat4x4(&lightView,
					XMMatrixLookToLH(
						position,
						XMLoadFloat3(&lookDir),
						XMLoadFloat3(&upDir)
					)
				);

				XMStoreFloat4x4(&lightProj,
					XMMatrixOrthographicLH(
						20,
						20,
						1.f,
						200.f
					)
				);

				break;
			}

			case LIGHT_TYPE_POINT:
			{
				break;
			}

			case LIGHT_TYPE_SPOT:
			{
				XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&lights[i].direction));

				XMVECTOR forward = XMVectorSet(0.f, 0.f, 1.f, 1.f);
				float dot;
				XMStoreFloat(&dot, XMVector3Dot(forward, lightDir));
				float angle = acos(dot);
				XMVECTOR axis = XMVector3Cross(forward, lightDir);

				Transform lightTransform = Transform();
				lightTransform.SetRotation(XMQuaternionRotationAxis(axis, angle));
				XMFLOAT3 lookDir = lightTransform.GetForward();
				XMFLOAT3 upDir = lightTransform.GetUp();

				XMStoreFloat4x4(&lightView,
					XMMatrixLookToLH(
						XMLoadFloat3(&lights[i].position),
						XMLoadFloat3(&lookDir),
						XMLoadFloat3(&upDir)
					)
				);

				XMStoreFloat4x4(&lightProj,
					XMMatrixPerspectiveFovLH(
						Rad2Deg(lights[i].spotFalloff),
						1.f,
						0.1f,
						lights[i].range
					)
				);

				break;
			}

			default:
				XMStoreFloat4x4(&lightView, XMMatrixIdentity());
				XMStoreFloat4x4(&lightProj, XMMatrixIdentity());
				break;
			}

			lightViewMatrices.push_back(lightView);
			lightProjMatrices.push_back(lightProj);

			context->OMSetRenderTargets(0, 0, dsvShadowMap.Get());
			context->RSSetState(shadowMapRasterizer.Get());

			D3D11_VIEWPORT lightViewport = {};
			lightViewport.TopLeftX = 0;
			lightViewport.TopLeftY = 0;
			lightViewport.Width = (float)shadowMapResolution;
			lightViewport.Height = (float)shadowMapResolution;
			lightViewport.MinDepth = 0.0f;
			lightViewport.MaxDepth = 1.0f;
			context->RSSetViewports(1, &lightViewport);

			shadowMapVertexShader->SetShader();
			shadowMapVertexShader->SetMatrix4x4("view", lightView);
			shadowMapVertexShader->SetMatrix4x4("proj", lightProj);
			context->PSSetShader(0, 0, 0);


			for (int i = 0; i < entities.size(); i++)
			{
				shadowMapVertexShader->SetMatrix4x4("world", entities[i]->GetTransform()->GetWorldMatrix());
				shadowMapVertexShader->CopyAllBufferData();
				entities[i]->GetMesh()->Draw();
			}

			shadowIndex++;
		}
	}
	
	// Reset rendering settings
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	context->RSSetState(0);

	D3D11_VIEWPORT standardViewport = {};
	standardViewport.TopLeftX = 0;
	standardViewport.TopLeftY = 0;
	standardViewport.Width = (float)windowWidth;
	standardViewport.Height = (float)windowHeight;
	standardViewport.MinDepth = 0.0f;
	standardViewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &standardViewport);

	context->RSSetState(0);
}