#include "Sky.h"

using namespace std;
using namespace DirectX;

Sky::Sky(
	std::shared_ptr<Mesh> mesh,
	const wchar_t* textureDdsPath,
	const wchar_t* vertexShaderPath,
	const wchar_t* pixelShaderPath,
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	)
	:
	mesh(mesh),
	textureSampler(sampler)
{
	CreateDDSTextureFromFile(device.Get(), textureDdsPath, nullptr, textureSrv.GetAddressOf());

	InitResources( vertexShaderPath, pixelShaderPath, device, context);
}

Sky::Sky(
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
	)
	:
	mesh(mesh),
	textureSampler(sampler)
{
	textureSrv = CreateCubemap(cubeRight, cubeLeft, cubeUp, cubeDown, cubeFront, cubeBack, device, context);
	
	InitResources(vertexShaderPath, pixelShaderPath, device, context);
}

Sky::~Sky()
{
}


void Sky::Draw(std::shared_ptr<Camera> camera, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Change necessary render states
	context->RSSetState(rasterizerState.Get());
	context->OMSetDepthStencilState(depthState.Get(), 0);

	// Prepare sky shaders for drawing
	vertexShader->SetShader();
	pixelShader->SetShader();

	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("proj", camera->GetProjectionMatrix());

	pixelShader->SetSamplerState("BasicSampler", textureSampler);
	pixelShader->SetShaderResourceView("SkyTexture", textureSrv);


	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();

	// Draw the mesh
	mesh->Draw();

	// Reset any render states changed
	context->RSSetState(nullptr);
	context->OMSetDepthStencilState(nullptr, 0);
}

// --------------------------------------------------------
// Author: Chris Cascioli
// Purpose: Creates a cube map on the GPU from 6 individual textures
// 
// - You are allowed to directly copy/paste this into your code base
//   for assignments, given that you clearly cite that this is not
//   code of your own design.
//
// - Note: This code assumes you’re putting the function in Game.cpp, 
//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
//   ComPtr called “device”.  Make any adjustments necessary for
//   your own implementation.
// --------------------------------------------------------
// --------------------------------------------------------
// Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face.  Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::CreateCubemap(
	const wchar_t* right,
	const wchar_t* left,
	const wchar_t* up,
	const wchar_t* down,
	const wchar_t* front,
	const wchar_t* back,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};
	CreateWICTextureFromFile(device.Get(), context.Get(), right, (ID3D11Resource**)textures[0].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), context.Get(), left, (ID3D11Resource**)textures[1].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), context.Get(), up, (ID3D11Resource**)textures[2].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), context.Get(), down, (ID3D11Resource**)textures[3].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), context.Get(), front, (ID3D11Resource**)textures[4].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), context.Get(), back, (ID3D11Resource**)textures[5].GetAddressOf(), 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first shader resource view
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array" with the TEXTURECUBE flag set.  
	// This is a special GPU resource format, NOT just a 
	// C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6;            // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0;       // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;   // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = mipCount;            // Compute mip maps for blurry reflections
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS; // This should be treated as a CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the final texture resource to hold the cube map
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	device->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,  // Which mip (zero, the starting highest quality)
			i,  // Which array element?
			mipCount); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		context->CopySubresourceRegion(
			cubeMapTexture.Get(),  // Destination resource
			subresource,           // Dest subresource index (one of the array elements)
			0, 0, 0,               // XYZ location of copy
			textures[i].Get(),     // Source resource
			0,                     // Source subresource index (we're assuming there's only one)
			0);                    // Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format;         // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = mipCount;        // Compute mip maps for blurry reflections
	srvDesc.TextureCube.MostDetailedMip = 0;  // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());

	// Auto generate mip maps of the skybox for blurrier versions to use for reflections
	context->GenerateMips(cubeSRV.Get());

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}

void Sky::InitResources(
	const wchar_t* vertexShaderPath,
	const wchar_t* pixelShaderPath,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	vertexShader = make_shared<SimpleVertexShader>(device, context, vertexShaderPath);
	pixelShader = make_shared<SimplePixelShader>(device, context, pixelShaderPath);

	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rastDesc, rasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthDesc, depthState.GetAddressOf());
}
