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
		1600,				// Width of the window's client area
		900,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	// Initialize the game's camera
	// Test scene
	XMFLOAT4 camStartRot;
	XMStoreFloat4(&camStartRot, XMQuaternionIdentity());
	camera = std::make_shared<Camera>(XMFLOAT3(5.0f, 10.0f, -30.0f), camStartRot);

	// Snowglobe scene
	/*XMFLOAT4 camStartRot;
	XMStoreFloat4(&camStartRot, XMQuaternionRotationAxis(XMVectorSet(1,0,0,0), Deg2Rad(30)));
	camera = std::make_shared<Camera>(XMFLOAT3(-2.0f, 22.0f, -28.3f), camStartRot);*/
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
	shadow = Shadow(2048, 1024, 4, device, context);

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
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/cube.obj", device, context));
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/sphere.obj", device, context));

	// Snowglobe scene
	/*meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/snowglobe.obj", device, context));
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/christmas_tree.obj", device, context));
	meshes.push_back(std::make_shared<Mesh>("../../Assets/Models/snowman.obj", device, context));*/
}

// Create a list of Game Entities to be rendered to the screen and initialize their starting transforms
// Not every mesh used in the game entities is centered at the origin so transformations are relative
void Game::CreateEntities()
{
	// Set up the Game Entity list using the pre-created meshes

	// Testing scene
	// Walls, Floor
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[0], GameEntityOptionFlags_MatWorldScale));
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[0], GameEntityOptionFlags_MatWorldScale));
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[0], GameEntityOptionFlags_MatWorldScale));
	// Test Sphere
	entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[0], GameEntityOptionFlags_MatWorldScale));

	// Snowglobe scene
	/*entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(meshes[2], materials[2]));
	entities.push_back(std::make_shared<GameEntity>(meshes[3], materials[3]));*/

	PositionGeometry();
}

void Game::CreateMaterials()
{
	// Default Material
	std::shared_ptr<Material> mDefault = std::make_shared<Material>("Default Grid", vertexShader, pixelShader);
	mDefault->SetAlbedo(srvDefaultGrid);
	mDefault->SetNormal(srvDefaultNormalMap);
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
	indirectLightIntensity = 0.5f;

	// Directional lights
	Light lDirectional = {};
	lDirectional.type = LIGHT_TYPE_DIRECTIONAL;
	lDirectional.direction = XMFLOAT3(-0.375f, -0.883f, 0.281f);
	lDirectional.color = XMFLOAT3(0.828f, 0.936f, 1.0f);
	lDirectional.intensity = 0.8f;
	lDirectional.castsShadows = 1;

	// Point lights
	Light lPoint = {};
	lPoint.type = LIGHT_TYPE_POINT;
	lPoint.position = XMFLOAT3(5.0f, 6.71f, -5.0f);
	lPoint.color = XMFLOAT3(1.0f, 0.311f, 0.169f);
	lPoint.intensity = 2.15f;
	lPoint.range = 20.0f;
	lPoint.castsShadows = 1;

	// Spot lights
	Light lSpot = {};
	lSpot.type = LIGHT_TYPE_SPOT;
	lSpot.position = XMFLOAT3(0, 3.0f, -2.0f);
	lSpot.direction = XMFLOAT3(0, 0, 1.0f);
	lSpot.color = XMFLOAT3(1, 1, 1);
	lSpot.intensity = 1;
	lSpot.range = 15;
	lSpot.spotFalloff = Deg2Rad(90.0f);
	lSpot.castsShadows = 1;

	lights.push_back(lDirectional);
	lights.push_back(lPoint);
	lights.push_back(lSpot);
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
	// Testing scene
	// Floor
	entities[0]->GetTransform()->SetPosition(0, -.2f, 0);
	entities[0]->GetTransform()->SetScale(100.0f, 0.2f, 100.0f);

	// Left wall
	entities[1]->GetTransform()->SetPosition(-10.0f, 10.0f, 0);
	entities[1]->GetTransform()->SetScale(1.0f, 10.0f, 100.0f);

	// Back wall
	entities[2]->GetTransform()->SetPosition(45.0f, 10.0f, 10.0f);
	entities[2]->GetTransform()->SetScale(55.0f, 10.0f, 1.0f);

	// Sphere
	entities[3]->GetTransform()->SetPosition(0, 3.0f, 0);

	// Snowglobe scene
	/*std::shared_ptr<GameEntity> snowglobe = entities[0];

	std::shared_ptr<GameEntity> christmasTree = entities[1];
	christmasTree->GetTransform()->SetScale(0.08f);
	christmasTree->GetTransform()->SetPosition(-1.58f, 5.44f, -5.2f);
	christmasTree->GetTransform()->RotatePitchYawRollWorld(0.f, Deg2Rad(-33.6f), 0.f);

	std::shared_ptr<GameEntity> snowman = entities[2];
	snowman->GetTransform()->SetScale(.5f);
	snowman->GetTransform()->SetPosition(3.47f, 5.29f, -4.98f);
	snowman->GetTransform()->RotatePitchYawRollWorld(0.f, Deg2Rad(-88.2f), 0.f);*/
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
	ImGuiMenus::EditScene(camera, entities, materials, lights, indirectLightIntensity, device, context);

	// Update the camera
	if (camera != 0)
	{
		camera->Update(deltaTime);
	}
	
	UpdateGeometry();

	shadow.Update(lights, device);
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

	shadow.Render(device, context, lights, entities, camera);
	
	// Reset rendering settings
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	D3D11_VIEWPORT standardViewport = {};
	standardViewport.TopLeftX = 0;
	standardViewport.TopLeftY = 0;
	standardViewport.Width = (float)windowWidth;
	standardViewport.Height = (float)windowHeight;
	standardViewport.MinDepth = 0.0f;
	standardViewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &standardViewport);
	context->RSSetState(0);

	// Render all objects in the scene
	for (int i = 0; i < entities.size(); i++)
	{
		std::shared_ptr<SimplePixelShader> ps = entities[i]->GetMaterial()->GetPixelShader();
		std::shared_ptr<SimpleVertexShader> vs = entities[i]->GetMaterial()->GetVertexShader();

		// Animated Pixel Shader needs the totalTime var
		ps->SetFloat("totalTime", totalTime);

		// Reflections and ambient light data are sampled from the skybox
		ps->SetShaderResourceView("SkyCubeMap", skybox->GetShaderResourceView());
		ps->SetInt("skyMipCount", skybox->GetMipCount());
		// Indirect lighting data is then scaled by a global intensity
		ps->SetFloat("indirectLightIntensity", indirectLightIntensity);

		if (lights.size() > 0)
		{
			// Lights data
			ps->SetData("lights", &lights[0], (int)lights.size() * sizeof(Light));
			ps->SetInt("lightCount", lights.size());

			// Shadow data
			// Send all of both types of Shadow Maps to the pixel shader through 2 Texture2DArrays stored in an SRVs
			ps->SetShaderResourceView("ShadowMapsCascade", shadow.GetDirLightShadowSrv());
			ps->SetInt("shadowCountCascade", shadow.GetNumCascadeShadowMaps());
			ps->SetShaderResourceView("ShadowMapsWorld", shadow.GetWorldPosLightShadowSrv());
			ps->SetInt("shadowCountWorld", shadow.GetNumWorldPosShadowMaps());
			ps->SetSamplerState("ShadowSampler", shadow.GetSampler());
		}

		if (shadow.GetWorldPosLightViews().size() > 0)
		{
			// The vertex shader needs the view and projection matrices used to create each Shadow Map
			// so that the pixel shader can interpret the Shadow Maps properly
			vs->SetData("lightViewsWorld", &shadow.GetWorldPosLightViews()[0], shadow.GetWorldPosLightViews().size() * sizeof(XMFLOAT4X4));
			vs->SetData("lightProjsWorld", &shadow.GetWorldPosLightProjs()[0], shadow.GetWorldPosLightProjs().size() * sizeof(XMFLOAT4X4));
		}

		if (shadow.GetDirLightViews().size() > 0)
		{
			vs->SetData("lightViewCascade", &shadow.GetDirLightViews()[0], shadow.GetDirLightViews().size() * sizeof(XMFLOAT4X4));
			vs->SetData("lightProjsCascade", &shadow.GetDirLightProjs()[0], shadow.GetDirLightProjs().size() * sizeof(XMFLOAT4X4));
		}

		entities[i]->Draw(context, camera);
	}

	// Draw the Skybox after each entity in the scene so that only the visible parts of the Skybox are rendered
	skybox->Draw(camera, context);

	// Draw ImGui UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Unbind shadow map srvs since they will used again as a depth buffer
	// before the next draw call. Still useful cleanup even though we create a new
	// srv resource with each shadow map render.
	ID3D11ShaderResourceView* nullSrvs[2] = {};
	context->PSSetShaderResources(4, 2, nullSrvs); // Index is defined in shader

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