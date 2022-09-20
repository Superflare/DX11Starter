#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> meshReference, VertexShaderExternalData vsData = VertexShaderExternalData());

	Transform* GetTransform();
	std::shared_ptr<Mesh> GetMesh();

	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer
	);

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	VertexShaderExternalData shaderData;
};

