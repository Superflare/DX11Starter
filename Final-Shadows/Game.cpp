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
	camera = std::make_shared<Camera>(XMFLOAT3(-2.0f, 22.0f, -28.3f), camStartRot, (float)1280 / 720);
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
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

	// Define and create the standard Texture Sampler State
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
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/snowglobe.obj", device, context));
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/christmas_tree.obj", device, context));
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/cube.obj", device, context));
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/snowman.obj", device, context));
}

// Create a list of Game Entities to be rendered to the screen and initialize their starting transforms
// Not every mesh used in the game entities is centered at the origin so transformations are relative
void Game::CreateEntities()
{
	// Set up the Game Entity list using the pre-created meshes
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[2]));
	entities.push_back(std::make_shared<GameEntity>(meshes[3], materials[3]));

	PositionGeometry();
}

void Game::CreateMaterials()
{
	// Default Material
	std::shared_ptr<Material> mDefault = std::make_shared<Material>("Default Grid", vertexShader, pixelShader);
	mDefault->SetAlbedo(srvDefaultGrid);
	mDefault->AddSampler("BasicSampler", texSampler);

	// Snowglobe
	std::shared_ptr<Material> mSnowglobe = std::make_shared<Material>("Snowglobe", vertexShader, pixelShader);
	mSnowglobe->SetAllPbrTextures(srvSnowglobe);
	mSnowglobe->AddSampler("BasicSampler", texSampler);

	// Christmas Tree
	std::shared_ptr<Material> mChristmasTree = std::make_shared<Material>("Christmas Tree", vertexShader, pixelShader, XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.9f, 0.f);
	mChristmasTree->SetAlbedo(srvChristmasTree);
	mChristmasTree->SetNormal(srvDefaultNormalMap);
	mChristmasTree->AddSampler("BasicSampler", texSampler);

	// Snowman
	std::shared_ptr<Material> mSnowman = std::make_shared<Material>("Snowman", vertexShader, pixelShader, XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.9f, 0.f);
	mSnowman->SetAlbedo(srvSnowman);
	mSnowman->SetNormal(srvDefaultNormalMap);
	mSnowman->AddSampler("BasicSampler", texSampler);

	materials.push_back(mDefault);
	materials.push_back(mSnowglobe);
	materials.push_back(mChristmasTree);
	materials.push_back(mSnowman);
}

void Game::SetupLights()
{
	// Directional lights
	Light lDirectional = {};
	lDirectional.type = LIGHT_TYPE_DIRECTIONAL;
	lDirectional.direction = XMFLOAT3(-0.375f, -0.883f, 0.281f);
	lDirectional.color = XMFLOAT3(0.828f, 0.936f, 1.0f);
	lDirectional.intensity = 1.01f;
	lDirectional.castsShadows = 1;

	// Point lights
	Light lPoint = {};
	lPoint.type = LIGHT_TYPE_POINT;
	lPoint.position = XMFLOAT3(1.57f, 6.71f, -2.86f);
	lPoint.color = XMFLOAT3(1.0f, 0.311f, 0.169f);
	lPoint.intensity = 2.15f;
	lPoint.range = 7.62f;
	lPoint.castsShadows = 1;

	lights.push_back(lDirectional);
	lights.push_back(lPoint);
}

// --------------------------------------------------------
// Creates reusable descriptions for Shadow Map resources
// --------------------------------------------------------
void Game::SetupShadows(int resolution)
{
	// Set class variables
	shadowMapResolution = resolution;
	numShadowMaps = 0;
	prevLightShadowSettings.clear();
	// Count the number of shadow maps required with the current lighting configuration
	for (int i = 0; i < lights.size(); i++)
	{
		// Point lights require 6 shadow maps (A Texture Cube) while all other lights only require 1
		if (lights[i].castsShadows == 1)
			lights[i].type == LIGHT_TYPE_POINT ? numShadowMaps += 6 : numShadowMaps++;
		
		// Store the current state's shadow casting settings
		// When the user changes whether a light casts shadows through the UI, it will be noticeable by comparing the
		// light's current properties to this vector
		prevLightShadowSettings.push_back(lights[i].castsShadows);
	}

	// The Depth Stencil View is always the same for every shadow map
	// but needs to be recreated each time it wants to render to a new texture, so keep the description in local scope
	shadowMapDsvDesc = {};
	shadowMapDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowMapDsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowMapDsvDesc.Texture2D.MipSlice = 0;

	// The description for an individual Texture2D Shadow Map
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

	// The description for an array of Shadow Maps
	shadowMapTextureArrayDesc = {};
	shadowMapTextureArrayDesc.Width = shadowMapResolution;
	shadowMapTextureArrayDesc.Height = shadowMapResolution;
	shadowMapTextureArrayDesc.MipLevels = 1;
	shadowMapTextureArrayDesc.ArraySize = numShadowMaps;
	shadowMapTextureArrayDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapTextureArrayDesc.SampleDesc.Count = 1;
	shadowMapTextureArrayDesc.SampleDesc.Quality = 0;
	shadowMapTextureArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapTextureArrayDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapTextureArrayDesc.CPUAccessFlags = 0;
	shadowMapTextureArrayDesc.MiscFlags = 0;

	// The description for the Shader Resource View that will hold the Shadow Map array
	shadowMapSrvDesc = {};
	shadowMapSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowMapSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shadowMapSrvDesc.Texture2DArray.ArraySize = numShadowMaps;
	shadowMapSrvDesc.Texture2DArray.MipLevels = 1;
	shadowMapSrvDesc.Texture2DArray.MostDetailedMip = 0;
	shadowMapSrvDesc.Texture2DArray.FirstArraySlice = 0;

	// Shadow Maps require a specific sampler to specify a comparison function, and the address mode
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
	device->CreateSamplerState(&shadowSamplerDesc, shadowMapSampler.ReleaseAndGetAddressOf());

	// Shadow Maps require a specific rasterizer to fix some common issues that arise from using Shadow Maps
	// (i.e. Noise and Self-Shadowing) 
	D3D11_RASTERIZER_DESC shadowRasterizerDesc = {};
	shadowRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	shadowRasterizerDesc.CullMode = D3D11_CULL_BACK;
	shadowRasterizerDesc.DepthClipEnable = true;
	shadowRasterizerDesc.DepthBias = 1000;
	shadowRasterizerDesc.DepthBiasClamp = 0.0f;
	shadowRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRasterizerDesc, shadowMapRasterizer.ReleaseAndGetAddressOf());

	// Create each individual Texture2D that will be used as a depth buffer and turned into a Shadow Map
	if (texShadowMaps.size() > 0)
		texShadowMaps.clear();

	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i].castsShadows == 1)
		{
			// Point lights require a Texture Cube while other lights only need 1 texture
			int iterations = lights[i].type == LIGHT_TYPE_POINT ? 6 : 1;
			for (int j = 0; j < iterations; j++)
			{
				Microsoft::WRL::ComPtr<ID3D11Texture2D> texShadowMap;
				device->CreateTexture2D(&shadowMapTextureDesc, 0, texShadowMap.ReleaseAndGetAddressOf());

				if (texShadowMap != 0)
				{
					texShadowMaps.push_back(texShadowMap);
				}
			}
		}
	}
}


void Game::LoadTextures()
{
	// Default Grid
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/default_albedo.png").c_str(),
		nullptr,
		srvDefaultGrid.GetAddressOf()
	);

	// Snowglobe
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/snowglobe_albedo.png").c_str(),
		nullptr,
		srvSnowglobe[0].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/snowglobe_normal.png").c_str(),
		nullptr,
		srvSnowglobe[1].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/snowglobe_roughness.png").c_str(),
		nullptr,
		srvSnowglobe[2].GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/snowglobe_metallic.png").c_str(),
		nullptr,
		srvSnowglobe[3].GetAddressOf()
	);

	// Christmas Tree
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/christmas_tree_albedo.png").c_str(),
		nullptr,
		srvChristmasTree.GetAddressOf()
	);

	// Snowman
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/snowman_albedo.png").c_str(),
		nullptr,
		srvSnowman.GetAddressOf()
	);

	// Default normal map
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/flat_normals.png").c_str(),
		nullptr,
		srvDefaultNormalMap.GetAddressOf()
	);

	// Create Skybox
	skybox = std::make_shared<Sky>(
		meshes[2],
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
	std::shared_ptr<GameEntity> snowglobe = entities[0];

	std::shared_ptr<GameEntity> christmasTree = entities[1];
	christmasTree->GetTransform()->SetScale(0.08f);
	christmasTree->GetTransform()->SetPosition(-1.58f, 5.44f, -5.2f);
	christmasTree->GetTransform()->RotatePitchYawRollWorld(0.f, Deg2Rad(-33.6f), 0.f);

	std::shared_ptr<GameEntity> snowman = entities[2];
	snowman->GetTransform()->SetScale(.5f);
	snowman->GetTransform()->SetPosition(3.47f, 5.29f, -4.98f);
	snowman->GetTransform()->RotatePitchYawRollWorld(0.f, Deg2Rad(-88.2f), 0.f);
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
	ImGuiMenus::EditScene(camera, entities, materials, &lights, device, context);

	// Update the camera
	if (camera != 0)
	{
		camera->Update(deltaTime);
	}
	
	UpdateGeometry();

	// Reset shadows when a light in the scene has started or stopped casting shadows
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
		std::shared_ptr<SimplePixelShader> ps = entities[i]->GetMaterial()->GetPixelShader();
		std::shared_ptr<SimpleVertexShader> vs = entities[i]->GetMaterial()->GetVertexShader();

		// Animated Pixel Shader needs the totalTime var
		ps->SetFloat("totalTime", totalTime);

		if (lights.size() > 0)
		{
			ps->SetData("lights", &lights[0], (int)lights.size() * sizeof(Light));
			// Send all of the Shadow Maps to the pixel shader through a Texture2DArray stored in an SRV
			ps->SetShaderResourceView("ShadowMaps", srvShadowMapArray);
			ps->SetSamplerState("ShadowSampler", shadowMapSampler);
		}

		if (lightViewMatrices.size() > 0)
		{
			// The vertex shader needs the view and projection matrices used to create each Shadow Map
			// so that the pixel shader can interpret the Shadow Maps properly
			vs->SetData("lightViews", &lightViewMatrices[0], numShadowMaps * sizeof(XMFLOAT4X4));
			vs->SetData("lightProjs", &lightProjMatrices[0], numShadowMaps * sizeof(XMFLOAT4X4));
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

// --------------------------------------------------------
// Handle all frame-by-frame shadow map implementation
// --------------------------------------------------------
void Game::RenderShadowMaps()
{
	lightViewMatrices.clear();
	lightProjMatrices.clear();

	// Set the renderer to the proper settings for only rendering depth buffers
	context->RSSetState(shadowMapRasterizer.Get());
	context->PSSetShader(0, 0, 0);

	D3D11_VIEWPORT lightViewport = {};
	lightViewport.TopLeftX = 0;
	lightViewport.TopLeftY = 0;
	lightViewport.Width = (float)shadowMapResolution;
	lightViewport.Height = (float)shadowMapResolution;
	lightViewport.MinDepth = 0.0f;
	lightViewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &lightViewport);

	if (numShadowMaps > 0)
	{
		device->CreateTexture2D(&shadowMapTextureArrayDesc, 0, texShadowMapArray.ReleaseAndGetAddressOf());
	}

	// Render scene from the pov of each light that casts shadows, and store the depth buffer as a shadow map
	int shadowIndex = 0;
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i].castsShadows == 1)
		{
			// This process is repeated 6 times for point lights
			int iterations = lights[i].type == LIGHT_TYPE_POINT ? 6 : 1;
			for (int j = 0; j < iterations; j++)
			{
				XMFLOAT4X4 lightView;
				XMFLOAT4X4 lightProj;

				// Create the view and projection matrices of the light based on its type
				switch (lights[i].type)
				{
					case LIGHT_TYPE_DIRECTIONAL:
					{
						XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&lights[i].direction));
						// Set the position of the directional light along the direction to the light starting from the world origin
						// While it makes sense for the light to be far away from the scene (the sun) in order to preserve shadow quality,
						// this position must be relatively close to the objects that will be mapped during this call
						XMVECTOR position = -20 * lightDir;

						// Setup the variables needed to rotate a new Transform object to match the light's direction
						XMVECTOR forward = XMVectorSet(0.f, 0.f, 1.f, 1.f);
						float dot;
						XMStoreFloat(&dot, XMVector3Dot(forward, lightDir));
						float angle = acos(dot);
						XMVECTOR axis = XMVector3Cross(forward, lightDir);

						// Use a Transform object to calculate the correct EyeDirection and UpDirection for the view matrix
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

						// Use an orthographic projection matrix because directional lights are meant to be
						// light coming from every possible position along the specified direction
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
						// A point light is omnidirectional, so to map objects to a depth buffer in all directions, 6 depth buffers must be used
						// Because of this, this code is repeated 6 times and each time uses a different axis direction pointing to one of the 6 faces of a cube
						XMVECTOR lightDir = XMLoadFloat3(&cubeFaceDirections[j]);

						// Setup the variables needed to rotate a new Transform object to match the light's direction
						XMVECTOR forward = XMVectorSet(0.f, 0.f, 1.f, 1.f);
						float dot;
						XMStoreFloat(&dot, XMVector3Dot(forward, lightDir));
						float angle = acos(dot);
						XMVECTOR axis;
						// Make sure the look direction and up direction are calculated correctly
						// even when the light's direction lines up with the forward vector
						if (dot != 1.f && dot != -1.f)
						{
							axis = XMVector3Cross(forward, lightDir);
						}
						else if (dot == 1.f)
						{
							axis = forward;
						}
						else
						{
							axis = XMVectorSet(0.f, 1.f, 0.f, 1.f);
						}

						// Use a Transform object to calculate the correct EyeDirection and UpDirection for the view matrix
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

						// Each projection matrix used is a frustum from the light's position to the entirety of one of its TextureCube faces
						// This projection matrix only extends as far as the light's range
						XMStoreFloat4x4(&lightProj,
							XMMatrixPerspectiveFovLH(
								90.f,
								1.f,
								0.1f,
								lights[i].range
							)
						);

						break;
					}

					case LIGHT_TYPE_SPOT:
					{
						XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&lights[i].direction));

						// Setup the variables needed to rotate a new Transform object to match the light's direction
						XMVECTOR forward = XMVectorSet(0.f, 0.f, 1.f, 1.f);
						float dot;
						XMStoreFloat(&dot, XMVector3Dot(forward, lightDir));
						float angle = acos(dot);
						XMVECTOR axis;
						// Make sure the look direction and up direction are calculated correctly
						// even when the light's direction lines up with the forward vector
						if (dot != 1.f && dot != -1.f)
						{
							axis = XMVector3Cross(forward, lightDir);
						}
						else if (dot == 1.f)
						{
							axis = forward;
						}
						else
						{
							axis = XMVectorSet(0.f, 0.f, -1.f, 1.f);
						}

						// Use a Transform object to calculate the correct EyeDirection and UpDirection for the view matrix
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

						// The spotlight is the easier projection matrix to create because its range and frustum match up exactly with its matrix
						// This matrix also uses an equal aspect ratio of 1
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

				// Clear the shadow map depth buffer
				if (dsvShadowMap != 0)
					context->ClearDepthStencilView(dsvShadowMap.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
				// Set the Depth Stencil View to render to the next Texture2D in our list
				device->CreateDepthStencilView(texShadowMaps[shadowIndex].Get(), &shadowMapDsvDesc, dsvShadowMap.ReleaseAndGetAddressOf());
				context->OMSetRenderTargets(0, 0, dsvShadowMap.Get());

				// Render all of the game entities in the scene to a depth buffer using a custom vertex shader
				for (int i = 0; i < entities.size(); i++)
				{
					shadowMapVertexShader->SetShader();
					shadowMapVertexShader->SetMatrix4x4("view", lightView);
					shadowMapVertexShader->SetMatrix4x4("proj", lightProj);
					shadowMapVertexShader->SetMatrix4x4("world", entities[i]->GetTransform()->GetWorldMatrix());
					shadowMapVertexShader->CopyAllBufferData();
					// Use the Mesh's draw method so no extra constant buffers or render settings are set
					entities[i]->GetMesh()->Draw();
				}

				// Copy the Texture2D depth buffer that was just rendered into the Texture2DArray that will be sent to the pixel shader
				// Calculate the subresource position to copy into
				unsigned int subresource = D3D11CalcSubresource(0, shadowIndex, 1);

				// Copy from the current individual Shadow Map to the Shadow Map Array
				context->CopySubresourceRegion(
					texShadowMapArray.Get(),
					subresource,
					0, 0, 0,
					texShadowMaps[shadowIndex].Get(),
					0,
					0
				);

				// Move on to the next Shadow Map
				shadowIndex++;
			}
		}
	}

	// Once all of the Shadow Maps have been rendered, and the texture array is stored with a copy of each,
	// use the texture array as our Shader Resource
	if (numShadowMaps > 0)
	{
		device->CreateShaderResourceView(texShadowMapArray.Get(), &shadowMapSrvDesc, srvShadowMapArray.ReleaseAndGetAddressOf());
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