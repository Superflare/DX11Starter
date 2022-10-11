#include "Transform.h"
#include <cmath>
using namespace DirectX;

Transform::Transform()
	: Transform(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT4X4(1,0,0,0,
								0,1,0,0,
								0,0,1,0,
								0,0,0,1)
	  )
{
}

Transform::Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4X4 rotationMatrix)
	:
	position(position),
	scale(scale),
	transformChanged(true),
	rotationChanged(true)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
	SetRotation(rotationMatrix);
	pitchYawRoll = GetRotationPitchYawRoll();
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

void Transform::SetRotation(DirectX::XMMATRIX rotationMatrix)
{
	XMFLOAT4X4 rotMat;
	XMStoreFloat4x4(&rotMat, rotationMatrix);

	rightVector = XMFLOAT3(rotMat._11, rotMat._12, rotMat._13);
	upVector = XMFLOAT3(rotMat._21, rotMat._22, rotMat._23);
	forwardVector = XMFLOAT3(rotMat._31, rotMat._32, rotMat._33);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(DirectX::XMFLOAT4X4 rotMat)
{
	rightVector = XMFLOAT3(rotMat._11, rotMat._12, rotMat._13);
	upVector = XMFLOAT3(rotMat._21, rotMat._22, rotMat._23);
	forwardVector = XMFLOAT3(rotMat._31, rotMat._32, rotMat._33);

	transformChanged = true;
	rotationChanged = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	SetRotation(XMMatrixRotationRollPitchYaw(pitch, yaw, roll));

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

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

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

void Transform::UpdatePitchYawRoll()
{
	if (rotationChanged)
	{
		//http://eecs.qmul.ac.uk/~gslabaugh/publications/euler.pdf
		//float yaw1;
		//float pitch1;
		//float roll1;
		//if (abs(forwardVector.x) != 1.0f)
		//{
		//	yaw1 = -asin(forwardVector.x);
		//	//float yaw2 = XM_PI - yaw1;
		//	pitch1 = atan2(forwardVector.y / cos(yaw1), forwardVector.z / cos(yaw1));
		//	//float pitch2 = atan2(forwardVector.y / cos(yaw2), forwardVector.z / cos(yaw2));
		//	roll1 = atan2(upVector.x / cos(yaw1), rightVector.x / cos(yaw1));
		//	//float roll2 = atan2(upVector.x / cos(yaw2), rightVector.x / cos(yaw2));
		//}
		//else 
		//{
		//	roll1 = 0;
		//	if (forwardVector.x == -1)
		//	{
		//		yaw1 = XM_PI / 2.0f;
		//		pitch1 = roll1 + atan2(rightVector.y, rightVector.z);
		//	}
		//	else
		//	{
		//		yaw1 = -XM_PI / 2.0f;
		//		pitch1 = -roll1 + atan2(-rightVector.y, -rightVector.z);
		//	}
		//}
		//pitchYawRoll.x = pitch1;
		//pitchYawRoll.y = yaw1;
		//pitchYawRoll.z = roll1;


		//https://stackoverflow.com/questions/60350349/directx-get-pitch-yaw-roll-from-xmmatrix
		pitchYawRoll.x = XMScalarASin(-forwardVector.y);

		XMVECTOR from(XMVectorSet(rightVector.y, forwardVector.x, 0.0f, 0.0f));
		XMVECTOR to(XMVectorSet(upVector.y, forwardVector.z, 0.0f, 0.0f));
		XMVECTOR result(XMVectorATan2(from, to));

		pitchYawRoll.z = XMVectorGetX(result);
		pitchYawRoll.y = XMVectorGetY(result);

		rotationChanged = false;
	}
}

DirectX::XMFLOAT3 Transform::GetRotationPitchYawRoll()
{
	UpdatePitchYawRoll();

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

DirectX::XMFLOAT3 Transform::GetRight()
{
	return rightVector;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	return upVector;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	return forwardVector;
}

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
		XMStoreFloat4x4(&worldInverseTransposeMatrix,
			XMMatrixInverse(nullptr, XMMatrixTranspose(world)));

		transformChanged = false;
	}
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

