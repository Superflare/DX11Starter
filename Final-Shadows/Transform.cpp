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
	qRotation = rotationQuat;
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

void Transform::SetRotation(DirectX::XMVECTOR q)
{
	XMStoreFloat4(&qRotation, q);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMFLOAT4 q)
{
	SetRotation(XMLoadFloat4(&q));
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	SetRotation(XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
}

void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll)
{
	SetRotation(XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));
}

void Transform::SetRotation(DirectX::XMMATRIX m)
{
	SetRotation(XMQuaternionRotationMatrix(m));
}

void Transform::SetRotation(DirectX::XMFLOAT4X4 m)
{
	SetRotation(XMQuaternionRotationMatrix(XMLoadFloat4x4(&m)));
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
	XMVECTOR moveRelative = XMVector3Rotate(moveAbsolute, XMLoadFloat4(&qRotation));

	XMStoreFloat3(&position, XMLoadFloat3(&position) + moveRelative);
	
	transformChanged = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 move)
{
	XMVECTOR moveAbsolute = XMLoadFloat3(&move);
	XMVECTOR moveRelative = XMVector3Rotate(moveAbsolute, XMLoadFloat4(&qRotation));

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

void Transform::RotatePitchYawRollLocal(float pitch, float yaw, float roll)
{
	XMVECTOR newRot = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	XMVECTOR combined = XMQuaternionMultiply(newRot, XMLoadFloat4(&qRotation));

	SetRotation(combined);
}

void Transform::RotatePitchYawRollLocal(DirectX::XMFLOAT3 pitchYawRoll)
{
	XMVECTOR newRot = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
	XMVECTOR combined = XMQuaternionMultiply(newRot, XMLoadFloat4(&qRotation));

	SetRotation(combined);
}

void Transform::RotatePitchYawRollWorld(float pitch, float yaw, float roll)
{
	XMVECTOR newRot = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	XMVECTOR combined = XMQuaternionMultiply(XMLoadFloat4(&qRotation), newRot);

	SetRotation(combined);
}

void Transform::RotatePitchYawRollWorld(DirectX::XMFLOAT3 pitchYawRoll)
{
	XMVECTOR newRot = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
	XMVECTOR combined = XMQuaternionMultiply(XMLoadFloat4(&qRotation), newRot);

	SetRotation(combined);
}

void Transform::RotateAxisLocal(float radians, DirectX::XMFLOAT3 rotateAround)
{
	XMVECTOR newRot = XMQuaternionRotationAxis(XMLoadFloat3(&rotateAround), radians);
	XMVECTOR combined = XMQuaternionMultiply(newRot, XMLoadFloat4(&qRotation));

	SetRotation(combined);
}

void Transform::RotateAxisWorld(float radians, DirectX::XMFLOAT3 rotateAround)
{
	XMVECTOR newRot = XMQuaternionRotationAxis(XMLoadFloat3(&rotateAround), radians);
	XMVECTOR combined = XMQuaternionMultiply(XMLoadFloat4(&qRotation), newRot);

	SetRotation(combined);
}

// ------------------------------------------------------------------
// Get Transform Properties
// ------------------------------------------------------------------
DirectX::XMFLOAT3 Transform::GetRight()
{
	UpdateWorldMatrix();

	return XMFLOAT3(worldMatrix._11, worldMatrix._12, worldMatrix._13);
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	UpdateWorldMatrix();

	return XMFLOAT3(worldMatrix._21, worldMatrix._22, worldMatrix._23);
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	UpdateWorldMatrix();

	return XMFLOAT3(worldMatrix._31, worldMatrix._32, worldMatrix._33);
}

DirectX::XMFLOAT4X4 Transform::GetRotationFloat4X4()
{
	XMFLOAT4X4 rotMat;
	XMFLOAT3 rightVector = GetRight();
	XMFLOAT3 upVector = GetUp();
	XMFLOAT3 forwardVector = GetForward();

	XMStoreFloat4x4(&rotMat, XMMatrixSet(rightVector.x, rightVector.y, rightVector.z, 0,
		upVector.x, upVector.y, upVector.z, 0,
		forwardVector.x, forwardVector.y, forwardVector.z, 0,
		0, 0, 0, 1));

	return rotMat;
}

DirectX::XMMATRIX Transform::GetRotationMatrix()
{
	XMFLOAT3 rightVector = GetRight();
	XMFLOAT3 upVector = GetUp();
	XMFLOAT3 forwardVector = GetForward();

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
		XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&qRotation));
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
		XMFLOAT3 rightVector = GetRight();
		XMFLOAT3 upVector = GetUp();
		XMFLOAT3 forwardVector = GetForward();

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