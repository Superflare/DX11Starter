#include "Camera.h"
#include "Input.h"
using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 startPos, DirectX::XMFLOAT4 startRot,
	float aspect, float fov,
	float nearClip, float farClip,
	float movSpeed, float mouseSpeed,
	ProjectionType projType)
	:
	fov(fov),
	nearClip(nearClip),
	farClip(farClip),
	movSpeed(movSpeed),
	mouseSpeed(mouseSpeed),
	projType(projType)
{
	transform = Transform(startPos, XMFLOAT3(1.0f, 1.0f, 1.0f), startRot);

	UpdateProjectionMatrix(aspect);
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 transformPos = transform.GetPosition();
	XMFLOAT3 transformForw = transform.GetForward();
	XMFLOAT3 transformUp = transform.GetUp();


	XMVECTOR pos = XMVectorSet(transformPos.x, transformPos.y, transformPos.z, 1.0f);
	XMVECTOR lookDir = XMVectorSet(transformForw.x, transformForw.y, transformForw.z, 1.0f);
	XMVECTOR upDir = XMVectorSet(transformUp.x, transformUp.y, transformUp.z, 1.0f);

	XMStoreFloat4x4(&viewMatrix, XMMatrixLookToLH(pos, lookDir, upDir));
}

void Camera::UpdateProjectionMatrix(float aspect)
{
	if (projType == Perspective)
	{
		XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(fov, aspect, nearClip, farClip));
	}
	else if (projType == Orthographic)
	{
		// TODO
	}
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projMatrix;
}

void Camera::Update(float dt)
{
	Input& input = Input::GetInstance();

#pragma region Keyboard Controls

	// WASD for simple movement controls
	if (input.KeyDown('W'))
	{
		transform.MoveRelative(0, 0, movSpeed * dt);
	}
	if (input.KeyDown('S'))
	{
		transform.MoveRelative(0, 0, -movSpeed * dt);
	}
	if (input.KeyDown('A'))
	{
		transform.MoveRelative(-movSpeed * dt, 0, 0);
	}
	if (input.KeyDown('D'))
	{
		transform.MoveRelative(movSpeed * dt, 0, 0);
	}

	// Hold E to move up and hold Q to move down
	if (input.KeyDown('E'))
	{
		transform.MoveRelative(0, movSpeed * dt, 0);
	}
	if (input.KeyDown('Q'))
	{
		transform.MoveRelative(0, -movSpeed * dt, 0);
	}

#pragma endregion

#pragma region Mouse Controls

	if (input.MouseLeftDown())
	{
		int cursorMovementX = input.GetMouseXDelta();
		int cursorMovementY = input.GetMouseYDelta();

		if (cursorMovementY > 0)
		{
			transform.Rotate(mouseSpeed * dt, 0, 0);

			// Clamp the rotation up and down to between directly up and directly down
			/*XMFLOAT4X4 curRotation = XMFLOAT4X4();
			XMStoreFloat4x4(&curRotation, XMMatrixRotationQuaternion(XMLoadFloat4(&transform.GetRotation())));
			if (curRotation._33 <= 0.0f)
			{
				transform.ReverseRotation(mouseSpeed * dt, transform.GetRight());
			}*/
		}
		else if (cursorMovementY < 0)
		{
			transform.Rotate(-mouseSpeed * dt, 0, 0);
		}

		if (cursorMovementX > 0)
		{
			transform.Rotate(0, mouseSpeed * dt, 0);
		}
		else if (cursorMovementX < 0)
		{
			transform.Rotate(0, -mouseSpeed * dt, 0);
		}
	}

#pragma endregion

	UpdateViewMatrix();
}
