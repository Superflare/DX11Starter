#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	Transform();
	Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4 rotation);

	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 pos);
	void SetScale(float x, float y, float z);
	void SetScale(float s);
	void SetScale(DirectX::XMFLOAT3 size);
	void SetRotation(float x, float y, float z, float w);
	void SetRotation(DirectX::XMFLOAT4 rot);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4 GetRotation();

	void UpdateWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();


	void MoveAbsolute(float x, float y, float z);
	void Scale(float x, float y, float z);
	void Scale(float s);
	void Scale(DirectX::XMFLOAT3 size);
	void Rotate(float radians, DirectX::XMFLOAT3 rotateAround = DirectX::XMFLOAT3(0, 0, -1.0f));

private:
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT4 rotation;

	bool transformChanged;
};

