#include <cmath>
#include "Transform.h"
using namespace DirectX;

Transform::Transform()
	: Transform(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT4(0,0,0,1)
	  )
{
}

Transform::Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4 rotationQuat)
	:
	position(position),
	scale(scale),
	transformChanged(true),
	rotationChanged(true)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
	SetRotation(rotationQuat);
	pitchYawRoll = GetRotationPitchYawRoll();
}

// ------------------------------------------------------------------
// Absolute Transform Changes
// ------------------------------------------------------------------
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

void Transform::SetRotation(DirectX::XMMATRIX m)
{
	XMFLOAT4X4 rotMat;
	XMStoreFloat4x4(&rotMat, m);

	rightVector = XMFLOAT3(rotMat._11, rotMat._12, rotMat._13);
	upVector = XMFLOAT3(rotMat._21, rotMat._22, rotMat._23);
	forwardVector = XMFLOAT3(rotMat._31, rotMat._32, rotMat._33);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMFLOAT4X4 m)
{
	rightVector = XMFLOAT3(m._11, m._12, m._13);
	upVector = XMFLOAT3(m._21, m._22, m._23);
	forwardVector = XMFLOAT3(m._31, m._32, m._33);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	SetRotation(XMMatrixRotationRollPitchYaw(pitch, yaw, roll));

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll)
{
	SetRotation(XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMFLOAT4 q)
{
	SetRotation(XMMatrixRotationQuaternion(XMLoadFloat4(&q)));

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMVECTOR q)
{
	SetRotation(XMMatrixRotationQuaternion(q));

	transformChanged = true;
	rotationChanged = true;
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

// ------------------------------------------------------------------
// Relative Transform Changes
// ------------------------------------------------------------------
void Transform::MoveRelative(float x, float y, float z)
{
	XMVECTOR moveAbsolute = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR rot = XMQuaternionRotationMatrix(GetRotationMatrix());
	XMVECTOR moveRelative = XMVector3Rotate(moveAbsolute, rot);

	XMStoreFloat3(&position, XMLoadFloat3(&position) + moveRelative);
	
	transformChanged = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 move)
{
	XMVECTOR moveAbsolute = XMLoadFloat3(&move);
	XMVECTOR rot = XMQuaternionRotationMatrix(GetRotationMatrix());
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
	XMVECTOR combinedQuaternion = XMQuaternionMultiply(newQuaternion, XMQuaternionRotationMatrix(GetRotationMatrix()));

	SetRotation(combinedQuaternion);

	pitchYawRoll.x += pitch;
	pitchYawRoll.y += yaw;
	pitchYawRoll.z += roll;

	transformChanged = true;
	rotationChanged = true;
}

void Transform::Rotate(float radians, DirectX::XMFLOAT3 rotateAround)
{
	XMVECTOR newQuaternion = XMQuaternionRotationNormal(XMLoadFloat3(&rotateAround), radians);
	XMVECTOR combinedQuaternion = XMQuaternionMultiply(newQuaternion, XMQuaternionRotationMatrix(GetRotationMatrix()));

	SetRotation(combinedQuaternion);

	transformChanged = true;
	rotationChanged = true;
}

// ------------------------------------------------------------------
// Get Transform Properties
// ------------------------------------------------------------------
DirectX::XMFLOAT4X4 Transform::GetRotationFloat4X4()
{
	XMFLOAT4X4 rotMat;
	XMStoreFloat4x4(&rotMat, XMMatrixSet(rightVector.x, rightVector.y, rightVector.z, 0,
		upVector.x, upVector.y, upVector.z, 0,
		forwardVector.x, forwardVector.y, forwardVector.z, 0,
		0, 0, 0, 1));

	return rotMat;
}

DirectX::XMMATRIX Transform::GetRotationMatrix()
{
	return XMMatrixSet(rightVector.x, rightVector.y, rightVector.z, 0,
		upVector.x, upVector.y, upVector.z, 0,
		forwardVector.x, forwardVector.y, forwardVector.z, 0,
		0, 0, 0, 1);
}

DirectX::XMFLOAT3 Transform::GetRotationPitchYawRoll()
{
	UpdatePitchYawRoll();

	return pitchYawRoll;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateWorldMatrix();

	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateWorldMatrix();

	return worldInverseTransposeMatrix;
}

// ------------------------------------------------------------------
// Update Class Fields When Transform Has Been Changed
// ------------------------------------------------------------------
void Transform::UpdateWorldMatrix()
{
	if (transformChanged)
	{
		XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX rotationMatrix = XMMatrixSet(rightVector.x, rightVector.y, rightVector.z, 0,
			upVector.x, upVector.y, upVector.z, 0,
			forwardVector.x, forwardVector.y, forwardVector.z, 0,
			0, 0, 0, 1);
		XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);
		XMMATRIX world = scaleMatrix * rotationMatrix * translationMatrix;

		XMStoreFloat4x4(&worldMatrix, world);
		XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(nullptr, XMMatrixTranspose(world)));

		transformChanged = false;
	}
}

void Transform::UpdatePitchYawRoll()
{
	if (rotationChanged)
	{
		// Solution derived from:
		// https://stackoverflow.com/questions/60350349/directx-get-pitch-yaw-roll-from-xmmatrix
		pitchYawRoll.x = XMScalarASin(-forwardVector.y);

		XMVECTOR from(XMVectorSet(rightVector.y, forwardVector.x, 0.0f, 0.0f));
		XMVECTOR to(XMVectorSet(upVector.y, forwardVector.z, 0.0f, 0.0f));
		XMVECTOR result(XMVectorATan2(from, to));

		pitchYawRoll.z = XMVectorGetX(result);
		pitchYawRoll.y = XMVectorGetY(result);

		rotationChanged = false;
	}
}