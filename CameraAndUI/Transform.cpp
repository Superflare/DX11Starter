#include "Transform.h"
#include <cmath>
using namespace DirectX;

Transform::Transform() : Transform(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f))
{
}

Transform::Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4 rotation)
	:
	position(position),
	scale(scale),
	rotation(rotation),
	transformChanged(true),
	rotationChanged(true)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
	pitchYawRoll = GetPitchYawRoll();
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;

	transformChanged = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 pos)
{
	position = pos;

	transformChanged = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	
	transformChanged = true;
}

void Transform::SetScale(float s)
{
	scale.x = s;
	scale.y = s;
	scale.z = s;

	transformChanged = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 size)
{
	scale = size;

	transformChanged = true;
}

void Transform::SetRotation(float x, float y, float z, float w)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
	rotation.w = w;

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMFLOAT4 rot)
{
	rotation = rot;

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMVECTOR rot)
{
	XMStoreFloat4(&rotation, rot);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	XMStoreFloat4(&rotation, XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));

	transformChanged = true;
	rotationChanged = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4 Transform::GetRotation()
{
	return rotation;
}

void Transform::UpdatePitchYawRoll()
{
	// Rotation around x-axis
	float sinr_cosp = 2 * (rotation.w * rotation.x + rotation.y * rotation.z);
	float cosr_cosp = 1 - 2 * (rotation.x * rotation.x * rotation.y * rotation.y);
	pitchYawRoll.x = atan2(sinr_cosp, cosr_cosp);

	// Rotation around y-axis
	float sinp = 2 * (rotation.w * rotation.y - rotation.z * rotation.x);
	if (abs(sinp) >= 1)
	{
		pitchYawRoll.y = copysign(XM_PIDIV2, sinp);
	}
	else
	{
		pitchYawRoll.y = asin(sinp);
	}

	// Rotation around z-axis
	float siny_cosp = 2 * (rotation.w * rotation.z + rotation.x * rotation.y);
	float cosy_cosp = 1 - 2 * (rotation.y * rotation.y + rotation.z * rotation.z);
	pitchYawRoll.z = atan2(siny_cosp, cosy_cosp);
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	if (rotationChanged)
	{
		UpdatePitchYawRoll();
		UpdateLocalAxes();
		rotationChanged = false;
	}

	return pitchYawRoll;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;

	transformChanged = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 move)
{
	position.x += move.x;
	position.y += move.y;
	position.z += move.z;

	transformChanged = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	XMVECTOR moveAbsolute = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR rot = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	XMVECTOR moveRelative = XMVector3Rotate(moveAbsolute, rot);

	XMStoreFloat3(&position, XMLoadFloat3(&position) + moveRelative);
	
	transformChanged = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 move)
{
	XMVECTOR moveAbsolute = XMLoadFloat3(&move);
	XMVECTOR rot = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	XMVECTOR moveRelative = XMVector3Rotate(moveAbsolute, rot);

	XMStoreFloat3(&position, XMLoadFloat3(&position) + moveRelative);

	transformChanged = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;

	transformChanged = true;
}

void Transform::Scale(float s)
{
	scale.x *= s;
	scale.y *= s;
	scale.z *= s;

	transformChanged = true;
}

void Transform::Scale(DirectX::XMFLOAT3 size)
{
	scale.x *= size.x;
	scale.y *= size.y;
	scale.z *= size.z;

	transformChanged = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR newQuaternion = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	XMVECTOR combinedQuaternion = XMQuaternionMultiply(XMLoadFloat4(&rotation), newQuaternion);

	XMStoreFloat4(&rotation, combinedQuaternion);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::Rotate(float radians, DirectX::XMFLOAT3 rotateAround)
{
	XMVECTOR newQuaternion = XMQuaternionRotationNormal(XMLoadFloat3(&rotateAround), radians);
	XMVECTOR combinedQuaternion = XMQuaternionMultiply(XMLoadFloat4(&rotation), newQuaternion);

	XMStoreFloat4(&rotation, combinedQuaternion);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::UpdateLocalAxes()
{
	XMVECTOR worldRight = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	XMVECTOR worldForward = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	XMVECTOR rot = XMLoadFloat4(&rotation);
	
	XMStoreFloat3(&rightVector, XMVector3Rotate(worldRight, rot));
	XMStoreFloat3(&upVector, XMVector3Rotate(worldUp, rot));
	XMStoreFloat3(&forwardVector, XMVector3Rotate(worldForward, rot));
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	if (rotationChanged)
	{
		UpdateLocalAxes();
		UpdatePitchYawRoll();
		rotationChanged = false;
	}

	return rightVector;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	if (rotationChanged)
	{
		UpdateLocalAxes();
		UpdatePitchYawRoll();
		rotationChanged = false;
	}

	return upVector;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	if (rotationChanged)
	{
		UpdateLocalAxes();
		UpdatePitchYawRoll();
		rotationChanged = false;
	}

	return forwardVector;
}

void Transform::UpdateWorldMatrix()
{
	XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
	XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX world = scaleMatrix * rotationMatrix * translationMatrix;

	XMStoreFloat4x4(&worldMatrix, world);
	XMStoreFloat4x4(&worldInverseTransposeMatrix,
		XMMatrixInverse(nullptr, XMMatrixTranspose(world)));
}


DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (transformChanged)
	{
		UpdateWorldMatrix();
		transformChanged = false;
	}

	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	if (transformChanged)
	{
		UpdateWorldMatrix();
		transformChanged = false;
	}

	return worldInverseTransposeMatrix;
}

