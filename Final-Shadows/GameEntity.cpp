#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> meshRef, std::shared_ptr<Material> mat, GameEntityOptionFlags_ optionFlags)
	:
	mesh(meshRef),
	material(mat),
	optionFlags(optionFlags)
{
	transform = Transform();
}

void GameEntity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<Camera> camera)
{
	HandleOptionFlags();

	// Set the active shaders to this entity's material
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	// Update each constant buffer's data
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());	// Strings here MUST match variable
	vs->SetMatrix4x4("view", camera->GetViewMatrix());		// names in the
	vs->SetMatrix4x4("proj", camera->GetProjectionMatrix()); // shader's cbuffer!
	vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());

	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	ps->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());

	material->Prepare();

	// Copy the constant buffer data from the CPU to the GPU
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	// Render this game entity's mesh
	mesh->Draw();
}

void GameEntity::HandleOptionFlags()
{
	if ((optionFlags & GameEntityOptionFlags_MatWorldScale) != 0)
	{
		material->SetTextureScale(1.0f / max(max(transform.GetScale().x, transform.GetScale().y), transform.GetScale().z));
	}
}
