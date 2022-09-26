#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> meshReference, VertexShaderExternalData vsData)
{
	transform = Transform();
	mesh = meshReference;
	shaderData = vsData;
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

void GameEntity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
	std::shared_ptr<Camera> camera)
{
	// Update constant buffer's world matrix
	shaderData.worldMatrix = transform.GetWorldMatrix();
	shaderData.viewMatrix = camera->GetViewMatrix();
	shaderData.projMatrix = camera->GetProjectionMatrix();

	// Copy the constant buffer data from the CPU to the GPU
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	memcpy(mappedBuffer.pData, &shaderData, sizeof(shaderData));

	context->Unmap(vsConstantBuffer.Get(), 0);

	context->VSSetConstantBuffers(
		0,    // Which slot (register) to bind the buffer to?
		1,    // How many are we activating? Can do multiple at once
		vsConstantBuffer.GetAddressOf()  // Array of buffers (or the address of one)
	);

	// Render this game entity's mesh
	mesh->Draw();
}
