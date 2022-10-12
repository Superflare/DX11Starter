#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	Transform();
	Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale,
		DirectX::XMFLOAT4 rotationQuat);
	
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 pos);
	void SetScale(float x, float y, float z);
	void SetScale(float s);
	void SetScale(DirectX::XMFLOAT3 size);
	void SetRotation(DirectX::XMMATRIX m);
	void SetRotation(DirectX::XMFLOAT4X4 m);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 pitchYawRoll);
	void SetRotation(DirectX::XMFLOAT4 q);
	void SetRotation(DirectX::XMVECTOR q);

	DirectX::XMFLOAT3 GetPosition() { return position; }
	DirectX::XMFLOAT3 GetScale() { return scale; }
	DirectX::XMFLOAT4X4 GetRotationFloat4X4();
	DirectX::XMMATRIX GetRotationMatrix();

	void UpdatePitchYawRoll();
	DirectX::XMFLOAT3 GetRotationPitchYawRoll();

	void UpdateWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();


	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 move);
	void MoveRelative(float x, float y, float z);
	void MoveRelative(DirectX::XMFLOAT3 move);
	void Scale(float x, float y, float z);
	void Scale(float s);
	void Scale(DirectX::XMFLOAT3 size);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(float radians, DirectX::XMFLOAT3 rotateAround = DirectX::XMFLOAT3(0, 0, -1.0f));
	
	DirectX::XMFLOAT3 GetRight() {return rightVector; }
	DirectX::XMFLOAT3 GetUp() { return upVector; }
	DirectX::XMFLOAT3 GetForward() { return forwardVector; }

private:
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 pitchYawRoll;

	DirectX::XMFLOAT3 rightVector;
	DirectX::XMFLOAT3 upVector;
	DirectX::XMFLOAT3 forwardVector;

	bool transformChanged;
	bool rotationChanged;
};

