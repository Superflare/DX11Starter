#include "Transform.h"
using namespace DirectX;

Transform::Transform() : Transform(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f))
{
}

Transform::Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4 rotation)
	:
	position(position),
	scale(scale),
	rotation(rotation),
	transformChanged(true)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
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
}

void Transform::SetRotation(DirectX::XMFLOAT4 rot)
{
	rotation = rot;

	transformChanged = true;
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
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR newQuaternion = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	XMVECTOR combinedQuaternion = XMQuaternionMultiply(XMLoadFloat4(&rotation), newQuaternion);

	XMStoreFloat4(&rotation, combinedQuaternion);

	transformChanged = true;
}

void Transform::Rotate(float radians, DirectX::XMFLOAT3 rotateAround)
{
	XMVECTOR newQuaternion = XMQuaternionRotationNormal(XMLoadFloat3(&rotateAround), radians);
	XMVECTOR combinedQuaternion = XMQuaternionMultiply(XMLoadFloat4(&rotation), newQuaternion);

	XMStoreFloat4(&rotation, combinedQuaternion);

	transformChanged = true;
}

void Transform::ReverseRotation(float radiansRotated, DirectX::XMFLOAT3 rotatedAround)
{
	XMVECTOR prevRotation = XMQuaternionRotationNormal(XMLoadFloat3(&rotatedAround), radiansRotated);
	XMVECTOR invertRotation = XMQuaternionInverse(prevRotation);
	XMVECTOR combinedQuaternion = XMQuaternionMultiply(XMLoadFloat4(&rotation), invertRotation);

	XMStoreFloat4(&rotation, combinedQuaternion);

	transformChanged = true;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	XMVECTOR worldRight = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rot = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	XMStoreFloat3(&rightVector, XMVector4Normalize(XMVector3Rotate(worldRight, rot)));

	return rightVector;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	XMVECTOR rot = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	XMStoreFloat3(&upVector, XMVector4Normalize(XMVector3Rotate(worldUp, rot)));

	return upVector;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	XMVECTOR worldForward = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	XMVECTOR rot = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
	XMStoreFloat3(&forwardVector, XMVector4Normalize(XMVector3Rotate(worldForward, rot)));

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

