#include "Shadow.h"
#include "Transform.h"
#include "Helpers.h"

Shadow::Shadow()
{
}

Shadow::Shadow(int cascadeHighestRes, int worldMapRes, int numCascades,
	const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context)
	:
	m_CascadeHighestRes(cascadeHighestRes),
	m_WorldMapRes(worldMapRes),
	m_NumCascades(numCascades)
{
	Init(device, context);
}

Shadow& Shadow::operator=(Shadow&& other)
{
	// All heap allocated memory is contained within smart pointers, so no additional work is required

	// Shallow copy all member fields
	m_CascadeHighestRes = other.m_CascadeHighestRes;
	m_WorldMapRes = other.m_WorldMapRes;
	m_NumCascades = other.m_NumCascades;
	m_NumWorldPosMaps = other.m_NumWorldPosMaps;
	m_lightSettings = other.m_lightSettings;
	m_VertexShader = other.m_VertexShader;
	// Example of a shared_ptr deep copy
	//m_VertexShader = std::make_shared<SimpleVertexShader>(other.m_VertexShader.get());
	m_Dsv = other.m_Dsv;
	// Example of a ComPtr deep copy
	//m_Dsv = Microsoft::WRL::ComPtr<ID3D11DepthStencilView>(other.m_Dsv.Get());
	m_DepthTexturesCascade = other.m_DepthTexturesCascade;
	m_DepthTexturesWorld = other.m_DepthTexturesWorld;
	m_TexArrayWorld = other.m_TexArrayWorld;
	m_TexArrayCascade = other.m_TexArrayCascade;
	m_CascadeSrv = other.m_CascadeSrv;
	m_WorldPosSrv = other.m_WorldPosSrv;
	m_Sampler = other.m_Sampler;
	m_Rasterizer = other.m_Rasterizer;
	m_TexDescWorld = other.m_TexDescWorld;
	m_TexArrayDescWorld = other.m_TexArrayDescWorld;
	m_SrvDescWorld = other.m_SrvDescWorld;
	m_TexDescCascade = other.m_TexDescCascade;
	m_TexArrayDescCascade = other.m_TexArrayDescCascade;
	m_SrvDescCascade = other.m_SrvDescCascade;
	m_DsvDesc = other.m_DsvDesc;
	m_CascadeLightViews = other.m_CascadeLightViews;
	m_CascadeLightProjs = other.m_CascadeLightProjs;
	m_WorldLightViews = other.m_WorldLightViews;
	m_WorldLightProjs = other.m_WorldLightProjs;

	return *this;
}

Shadow::~Shadow()
{
}

// -----------------------------------------------------------------------------------
// Initializes variables and creates reusable descriptions for Shadow Map resources
// -----------------------------------------------------------------------------------
void Shadow::Init(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context)
{
	m_NumWorldPosMaps = 0;

	m_VertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"ShadowMapVertexShader.cso").c_str());

	// The description for an individual Texture2D Shadow Map
	// Point and spot light desc
	m_TexDescWorld = {};
	m_TexDescWorld.Width = m_WorldMapRes;
	m_TexDescWorld.Height = m_WorldMapRes;
	m_TexDescWorld.MipLevels = 1;
	m_TexDescWorld.ArraySize = 1;
	m_TexDescWorld.Format = DXGI_FORMAT_R32_TYPELESS;
	m_TexDescWorld.SampleDesc.Count = 1;
	m_TexDescWorld.SampleDesc.Quality = 0;
	m_TexDescWorld.Usage = D3D11_USAGE_DEFAULT;
	m_TexDescWorld.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	m_TexDescWorld.CPUAccessFlags = 0;
	m_TexDescWorld.MiscFlags = 0;

	// Directional cascade desc
	m_TexDescCascade = m_TexDescWorld;
	m_TexDescCascade.Width = m_CascadeHighestRes;
	m_TexDescCascade.Height = m_CascadeHighestRes;

	// The description for an array of Shadow Maps (compilation held by srv)
	// Point and spot light desc
	m_TexArrayDescWorld = {};
	m_TexArrayDescWorld.Width = m_WorldMapRes;
	m_TexArrayDescWorld.Height = m_WorldMapRes;
	m_TexArrayDescWorld.MipLevels = 1;
	m_TexArrayDescWorld.ArraySize = 0;
	m_TexArrayDescWorld.Format = DXGI_FORMAT_R32_TYPELESS;
	m_TexArrayDescWorld.SampleDesc.Count = 1;
	m_TexArrayDescWorld.SampleDesc.Quality = 0;
	m_TexArrayDescWorld.Usage = D3D11_USAGE_DEFAULT;
	m_TexArrayDescWorld.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	m_TexArrayDescWorld.CPUAccessFlags = 0;
	m_TexArrayDescWorld.MiscFlags = 0;

	// Directional cascade desc
	m_TexArrayDescCascade = m_TexArrayDescWorld;
	m_TexArrayDescCascade.Width = m_CascadeHighestRes;
	m_TexArrayDescCascade.Height = m_CascadeHighestRes;

	// The description for the Shader Resource View that will hold the Shadow Map array
	// Point and spot light desc
	m_SrvDescWorld = {};
	m_SrvDescWorld.Format = DXGI_FORMAT_R32_FLOAT;
	m_SrvDescWorld.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	m_SrvDescWorld.Texture2DArray.ArraySize = 0;
	m_SrvDescWorld.Texture2DArray.MipLevels = 1;
	m_SrvDescWorld.Texture2DArray.MostDetailedMip = 0;
	m_SrvDescWorld.Texture2DArray.FirstArraySlice = 0;

	// Directional cascade desc
	m_SrvDescCascade = m_SrvDescWorld;

	// The Depth Stencil View is always the same for every shadow map
	// but needs to be recreated each time it wants to render to a new texture, so keep the description in local scope
	m_DsvDesc = {};
	m_DsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	m_DsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	m_DsvDesc.Texture2D.MipSlice = 0;

	// Shadow Maps require a specific sampler to specify a comparison function, and the address mode
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Trilinear filtering
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&samplerDesc, m_Sampler.GetAddressOf());

	// Shadow Maps require a specific rasterizer to fix some common issues that arise from using Shadow Maps
	// (i.e. Noise and Self-Shadowing) 
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.DepthBias = 1000;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&rasterizerDesc, m_Rasterizer.GetAddressOf());
}

void Shadow::SetupTextures(const std::vector<Light>& lights, const Microsoft::WRL::ComPtr<ID3D11Device>& device)
{
	// Fill both Texture2D vectors with all the textures that will be needed for shadow maps before the first draw call
	if (m_DepthTexturesWorld.empty() || m_DepthTexturesCascade.empty())
	{
		for (auto l : lights)
		{
			// Setup a texture resource for every light in the scene
			int iterations = l.type == LIGHT_TYPE_POINT ? 6 : l.type == LIGHT_TYPE_DIRECTIONAL ? m_NumCascades : 1;

			for (int j = 0; j < iterations; j++)
			{
				Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
				D3D11_TEXTURE2D_DESC desc = l.type == LIGHT_TYPE_DIRECTIONAL ? m_TexDescCascade : m_TexDescWorld;
				device->CreateTexture2D(&desc, 0, tex.GetAddressOf());

				if (l.type == LIGHT_TYPE_DIRECTIONAL)
					m_DepthTexturesCascade.push_back(tex);
				else		
					m_DepthTexturesWorld.push_back(tex);
			}
		}
	}

	// Now that we know the count of lights and their required shadow maps, setup Texture2DArrays and their Srvs
	// Completing all this work here means these do not need to be touched again unless the texture size or count changes
	m_TexArrayDescWorld.ArraySize = m_NumWorldPosMaps;
	m_SrvDescWorld.Texture2DArray.ArraySize = m_NumWorldPosMaps;
	m_TexArrayDescCascade.ArraySize = m_NumCascades;
	m_SrvDescCascade.Texture2DArray.ArraySize = m_NumCascades;

	device->CreateTexture2D(&m_TexArrayDescWorld, 0, m_TexArrayWorld.GetAddressOf());
	device->CreateShaderResourceView(m_TexArrayWorld.Get(), &m_SrvDescWorld, m_WorldPosSrv.GetAddressOf());
	device->CreateTexture2D(&m_TexArrayDescCascade, 0, m_TexArrayCascade.GetAddressOf());
	device->CreateShaderResourceView(m_TexArrayCascade.Get(), &m_SrvDescCascade, m_CascadeSrv.GetAddressOf());
}

void Shadow::Update(const std::vector<Light>& lights, const Microsoft::WRL::ComPtr<ID3D11Device>& device)
{
	// First update setup
	if (m_lightSettings.empty())
	{
		for (auto l : lights)
		{
			// Point lights require 6 shadow maps (A Texture Cube) while all other lights only require 1
			if (l.castsShadows == 1)
			{
				m_NumWorldPosMaps = m_NumWorldPosMaps + (l.type == LIGHT_TYPE_POINT ? 6 : l.type == LIGHT_TYPE_SPOT ? 1 : 0);
			}

			// Store the current state's shadow casting settings
			// When the user changes whether a light casts shadows through the UI, it will be noticeable by comparing the
			// light's current properties to this vector
			m_lightSettings.push_back(l.castsShadows);
		}

		// Shadow map texture setup, only used on first update
		SetupTextures(lights, device);
		return;
	}

	// Subsequent updates
	for (int i = 0; i < lights.size(); i++)
	{
		// New shadow setting
		if (lights[i].castsShadows != m_lightSettings[i])
		{
			// Update setting array
			m_lightSettings[i] = lights[i].castsShadows;

			// This light has recently turned on shadows
			if (lights[i].castsShadows)
			{
				m_NumWorldPosMaps = m_NumWorldPosMaps + (lights[i].type == LIGHT_TYPE_POINT ? 6 : lights[i].type == LIGHT_TYPE_SPOT ? 1 : 0);
				UpdateTexNumber(m_NumCascades, m_NumWorldPosMaps, device);
			}
			// This light has recently turned off shadows
			else
			{
				m_NumWorldPosMaps = m_NumWorldPosMaps - (lights[i].type == LIGHT_TYPE_POINT ? 6 : lights[i].type == LIGHT_TYPE_SPOT ? 1 : 0);
			}
		}
	}
}

void Shadow::UpdateTexNumber(int numCascades, int numWorldPosMaps, const Microsoft::WRL::ComPtr<ID3D11Device>& device)
{
	// Exit early if called before resources are initialized
	if (m_DepthTexturesCascade.empty() || m_DepthTexturesWorld.empty() ||
		!m_TexArrayCascade || !m_CascadeSrv || !m_TexArrayWorld || !m_WorldPosSrv)
	{
		return;
	}

	// Directional light cascade textures ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Texture array counts only need to change when they are increased
	// (i.e. A texture array only using 3 of its 5 array slices will still work fine and be read properly)
	if (numCascades > m_TexArrayDescCascade.ArraySize)
	{
		// Increase the number of individual Texture2Ds
		for (int i = 0; i < numCascades - m_TexArrayDescCascade.ArraySize; i++)
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			device->CreateTexture2D(&m_TexDescCascade, 0, tex.GetAddressOf());
			m_DepthTexturesCascade.push_back(tex);
		}

		// Update the Texture2DArray and its Srv to include the new array slices
		m_TexArrayDescCascade.ArraySize = numCascades;
		m_SrvDescCascade.Texture2DArray.ArraySize = numCascades;

		device->CreateTexture2D(&m_TexArrayDescCascade, 0, m_TexArrayCascade.ReleaseAndGetAddressOf());
		device->CreateShaderResourceView(m_TexArrayCascade.Get(), &m_SrvDescCascade, m_CascadeSrv.ReleaseAndGetAddressOf());
	}

	// Point and spot light world position textures ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Texture array counts only need to change when they are increased
	// (i.e. A texture array only using 3 of its 5 array slices will still work fine and be read properly)
	if (numWorldPosMaps > m_TexArrayDescWorld.ArraySize)
	{
		// Increase the number of individual Texture2Ds
		for (int i = 0; i < numWorldPosMaps - m_TexArrayDescWorld.ArraySize; i++)
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			device->CreateTexture2D(&m_TexDescWorld, 0, tex.GetAddressOf());
			m_DepthTexturesCascade.push_back(tex);
		}

		// Update the Texture2DArray and its Srv to include the new array slices
		m_TexArrayDescWorld.ArraySize = numWorldPosMaps;
		m_SrvDescWorld.Texture2DArray.ArraySize = numWorldPosMaps;

		device->CreateTexture2D(&m_TexArrayDescWorld, 0, m_TexArrayWorld.ReleaseAndGetAddressOf());
		device->CreateShaderResourceView(m_TexArrayWorld.Get(), &m_SrvDescWorld, m_WorldPosSrv.ReleaseAndGetAddressOf());
	}
}

void Shadow::Resize(int cascadeHighestRes, int worldMapRes, const Microsoft::WRL::ComPtr<ID3D11Device>& device)
{
	// Directional light cascade textures ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	m_CascadeHighestRes = cascadeHighestRes;
	// Resize texture descriptions
	m_TexDescCascade.Width = cascadeHighestRes;
	m_TexDescCascade.Height = cascadeHighestRes;
	m_TexArrayDescCascade.Width = cascadeHighestRes;
	m_TexArrayDescCascade.Height = cascadeHighestRes;

	// Use the resized descriptions to remake the textures affected
	for (auto t : m_DepthTexturesCascade)
	{
		device->CreateTexture2D(&m_TexDescCascade, 0, t.ReleaseAndGetAddressOf());
	}

	if (m_TexArrayCascade)
		device->CreateTexture2D(&m_TexArrayDescCascade, 0, m_TexArrayCascade.ReleaseAndGetAddressOf());

	// Point and spot light world position textures ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	m_WorldMapRes = worldMapRes;
	// Resize texture descriptions
	m_TexDescWorld.Width = worldMapRes;
	m_TexDescWorld.Height = worldMapRes;
	m_TexArrayDescWorld.Width = worldMapRes;
	m_TexArrayDescWorld.Height = worldMapRes;

	// Use the resized descriptions to remake the textures affected
	for (auto t : m_DepthTexturesWorld)
	{
		device->CreateTexture2D(&m_TexDescWorld, 0, t.ReleaseAndGetAddressOf());
	}

	if (m_TexArrayWorld)
		device->CreateTexture2D(&m_TexArrayDescWorld, 0, m_TexArrayWorld.ReleaseAndGetAddressOf());
}

// --------------------------------------------------------
// Handle all frame-by-frame shadow map implementation
// --------------------------------------------------------
void Shadow::Render
(
	const Microsoft::WRL::ComPtr<ID3D11Device>& device,
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
	const std::vector<Light>& lights,
	const std::vector<std::shared_ptr<GameEntity>>& entities,
	const std::shared_ptr<Camera>& cam
)
{
	InternalRenderCascade(device, context, lights, entities, cam);
	InternalRenderWorldPos(device, context, lights, entities);
}

// --------------------------------------------------------
// Handle all frame-by-frame shadow map implementation for the cascaded shadows
// of the 1 eligible directional light in the scene
// --------------------------------------------------------
void Shadow::InternalRenderCascade
(
	const Microsoft::WRL::ComPtr<ID3D11Device>& device,
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
	const std::vector<Light>& lights,
	const std::vector<std::shared_ptr<GameEntity>>& entities,
	const std::shared_ptr<Camera>& cam
)
{
	using namespace DirectX;

	m_CascadeLightViews.clear();
	m_CascadeLightProjs.clear();

	// Set the renderer to the proper settings for only rendering depth buffers
	context->RSSetState(m_Rasterizer.Get());
	context->PSSetShader(0, 0, 0);

	D3D11_VIEWPORT lightViewport = {};
	lightViewport.TopLeftX = 0;
	lightViewport.TopLeftY = 0;
	lightViewport.Width = (float)m_CascadeHighestRes;
	lightViewport.Height = (float)m_CascadeHighestRes;
	lightViewport.MinDepth = 0.0f;
	lightViewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &lightViewport);

	int texIdx = 0;
	for (auto l : lights)
	{
		// This process is repeated for each directional light shadow cascade level
		int iterations = l.type == LIGHT_TYPE_DIRECTIONAL ? m_NumCascades : 0;

		for (int j = 0; j < iterations; j++)
		{
			if (!CreateLightMatrices(l, m_CascadeLightViews, m_CascadeLightProjs, cam, j))
			{
				texIdx += iterations;
				break;
			}

			// Set the Depth Stencil View to render to the next Texture2D in our list
			device->CreateDepthStencilView(m_DepthTexturesCascade[texIdx].Get(), &m_DsvDesc, m_Dsv.ReleaseAndGetAddressOf());
			// Clear the shadow map depth buffer
			context->ClearDepthStencilView(m_Dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
			context->OMSetRenderTargets(0, 0, m_Dsv.Get());

			// Render all of the game entities in the scene to a depth buffer using a custom vertex shader
			for (auto e : entities)
			{
				m_VertexShader->SetShader();
				m_VertexShader->SetMatrix4x4("view", m_CascadeLightViews[m_CascadeLightViews.size() - 1]);
				m_VertexShader->SetMatrix4x4("proj", m_CascadeLightProjs[m_CascadeLightProjs.size() - 1]);
				m_VertexShader->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
				m_VertexShader->CopyAllBufferData();
				// Use the Mesh's draw method so no extra constant buffers or render settings are set
				e->GetMesh()->Draw();
			}

			// Copy the Texture2D depth buffer that was just rendered into the Texture2DArray that will be sent to the pixel shader
			// Calculate the subresource position to copy into
			unsigned int subresource = D3D11CalcSubresource(0, m_CascadeLightProjs.size() - 1, 1);

			// Copy from the current individual Shadow Map to the Shadow Map Array
			context->CopySubresourceRegion(
				m_TexArrayCascade.Get(),
				subresource,
				0, 0, 0,
				m_DepthTexturesCascade[texIdx].Get(),
				0,
				0
			);

			texIdx++;
		}
	}
}

// --------------------------------------------------------
// Handle all frame-by-frame shadow map implementation for the lights with
// positions in world space (point and spot lights) with no cascading
// --------------------------------------------------------
void Shadow::InternalRenderWorldPos
(
	const Microsoft::WRL::ComPtr<ID3D11Device>& device,
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context,
	const std::vector<Light>& lights,
	const std::vector<std::shared_ptr<GameEntity>>& entities
)
{
	using namespace DirectX; using std::vector;

	m_WorldLightViews.clear();
	m_WorldLightProjs.clear();

	// Set the renderer to the proper settings for only rendering depth buffers
	context->RSSetState(m_Rasterizer.Get());
	context->PSSetShader(0, 0, 0);

	D3D11_VIEWPORT lightViewport = {};
	lightViewport.TopLeftX = 0;
	lightViewport.TopLeftY = 0;
	lightViewport.Width = (float)m_WorldMapRes;
	lightViewport.Height = (float)m_WorldMapRes;
	lightViewport.MinDepth = 0.0f;
	lightViewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &lightViewport);

	int texIdx = 0;
	for (auto l : lights)
	{
		// This process is repeated 6 times for point lights, skip over directional lights as they do not have a world position
		int iterations = l.type == LIGHT_TYPE_POINT ? 6 : l.type == LIGHT_TYPE_DIRECTIONAL ? 0 : 1;

		for (int j = 0; j < iterations; j++)
		{
			if (!CreateLightMatrices(l, m_WorldLightViews, m_WorldLightProjs, nullptr, j))
			{
				texIdx += iterations;
				break;
			}

			// Set the Depth Stencil View to render to the next Texture2D in our list
			device->CreateDepthStencilView(m_DepthTexturesWorld[texIdx].Get(), &m_DsvDesc, m_Dsv.ReleaseAndGetAddressOf());
			// Clear the shadow map depth buffer
			context->ClearDepthStencilView(m_Dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
			context->OMSetRenderTargets(0, 0, m_Dsv.Get());

			// Render all of the game entities in the scene to a depth buffer using a custom vertex shader
			for (auto e : entities)
			{
				m_VertexShader->SetShader();
				m_VertexShader->SetMatrix4x4("view", m_WorldLightViews[m_WorldLightViews.size() - 1]);
				m_VertexShader->SetMatrix4x4("proj", m_WorldLightProjs[m_WorldLightProjs.size() - 1]);
				m_VertexShader->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
				m_VertexShader->CopyAllBufferData();
				// Use the Mesh's draw method so no extra constant buffers or render settings are set
				e->GetMesh()->Draw();
			}

			// Copy the Texture2D depth buffer that was just rendered into the Texture2DArray that will be sent to the pixel shader
			// Calculate the subresource position to copy into
			unsigned int subresource = D3D11CalcSubresource(0, m_WorldLightViews.size() - 1, 1);

			// Copy from the current individual Shadow Map to the Shadow Map Array
			context->CopySubresourceRegion(
				m_TexArrayWorld.Get(),
				subresource,
				0, 0, 0,
				m_DepthTexturesWorld[texIdx].Get(),
				0,
				0
			);

			texIdx++;
		}
	}
}

bool Shadow::CreateLightMatrices
(
	const Light& l,
	std::vector<DirectX::XMFLOAT4X4>& view,
	std::vector<DirectX::XMFLOAT4X4>& proj,
	const std::shared_ptr<Camera>& cam,
	int idx
)
{
	using namespace DirectX;

	if (l.castsShadows == 0)
		return false;

	XMFLOAT4X4 lightView;
	XMFLOAT4X4 lightProj;

	// Create the view and projection matrices of the light based on its type
	switch (l.type)
	{
		case LIGHT_TYPE_DIRECTIONAL:
		{
			if (view.empty())
			{
				XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&l.direction));
				// Set the position of the directional light along the direction to the light starting from the world origin
				// While it makes sense for the light to be far away from the scene (the sun) in order to preserve shadow quality,
				// this position must be relatively close to the objects that will be mapped during this call
				XMFLOAT3 camPos = cam->GetTransform()->GetPosition();
				camPos.y += 15.0f;
				XMVECTOR position = -65.0f * lightDir + XMLoadFloat3(&camPos);

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

				XMStoreFloat4x4(&lightView, XMMatrixLookToLH(position, XMLoadFloat3(&lookDir), XMLoadFloat3(&upDir)));
			}

			// Use an orthographic projection matrix because directional lights are meant to be
			// light coming from every possible position along the specified direction

			// Use a different scale of projection matrix for each shadow cascade
			// Lower cascades are higher quality and lower size, Higher cascades are higher sized so also lower quality
			switch (idx)
			{
				case 0:
					XMStoreFloat4x4(&lightProj, XMMatrixOrthographicLH(20, 20, 1.f, 250.f));
					break;
				case 1:
					XMStoreFloat4x4(&lightProj, XMMatrixOrthographicLH(65, 65, 1.f, 250.f));
					break;
				case 2:
					XMStoreFloat4x4(&lightProj, XMMatrixOrthographicLH(170, 170, 1.f, 250.f));
					break;
				case 3:
					XMStoreFloat4x4(&lightProj, XMMatrixOrthographicLH(400, 400, 1.f, 250.f));
					break;
				default:
					XMStoreFloat4x4(&lightProj, XMMatrixOrthographicLH(400, 400, 1.f, 250.f));
			}

			break;
		}

		case LIGHT_TYPE_POINT:
		{
			// A point light is omnidirectional, so to map objects to a depth buffer in all directions, 6 depth buffers must be used
			// Because of this, this code is repeated 6 times and each time uses a different axis direction pointing to one of the 6 faces of a cube
			XMVECTOR lightDir = XMLoadFloat3(&cubeFaceDirections[idx]);

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
				XMMatrixLookToLH(XMLoadFloat3(&l.position), XMLoadFloat3(&lookDir), XMLoadFloat3(&upDir))
			);

			// Each projection matrix used is a frustum from the light's position to the entirety of one of its TextureCube faces
			// This projection matrix only extends as far as the light's range
			XMStoreFloat4x4(&lightProj, XMMatrixPerspectiveFovLH(Deg2Rad(90.1f), 1.f, 0.1f, l.range));

			break;
		}

		case LIGHT_TYPE_SPOT:
		{
			XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&l.direction));

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
				XMMatrixLookToLH(XMLoadFloat3(&l.position), XMLoadFloat3(&lookDir), XMLoadFloat3(&upDir))
			);

			// The spotlight is the easier projection matrix to create because its range and frustum match up exactly with its matrix
			// This matrix also uses an equal aspect ratio of 1
			XMStoreFloat4x4(&lightProj, XMMatrixPerspectiveFovLH(l.spotFalloff, 1.f, 0.1f, l.range));

			break;
		}

		default:
		{
			XMStoreFloat4x4(&lightView, XMMatrixIdentity());
			XMStoreFloat4x4(&lightProj, XMMatrixIdentity());
			break;
		}
	}

	// Only push 1 light view for a directional light cascade as it always stays the same
	if (l.type != LIGHT_TYPE_DIRECTIONAL || view.empty())
		view.push_back(lightView);
	proj.push_back(lightProj);

	return true;
}
