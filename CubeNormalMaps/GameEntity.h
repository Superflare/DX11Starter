#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> meshRef, std::shared_ptr<Material> mat);

	Transform* GetTransform() { return &transform; }
	std::shared_ptr<Mesh> GetMesh() { return mesh; }
	std::shared_ptr<Material> GetMaterial() { return material; }

	void SetTransform(Transform t) { transform = t; }
	void SetMesh(std::shared_ptr<Mesh> m) { mesh = m; }
	void SetMaterial(std::shared_ptr<Material> m) { material = m; }

	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camera
	);

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

