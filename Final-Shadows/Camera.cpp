#include "Camera.h"
#include "Helpers.h"
#include "Input.h"
using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 startPos, DirectX::XMFLOAT4 startRot,
	float aspect, float fov,
	float nearClip, float farClip,
	float movSpeed, float mouseSpeed,
	ProjectionType projType)
	:
	aspect(aspect),
	fov(fov),
	orthoWidth(1280),
	orthoHeight(orthoWidth / aspect),
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

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	if (projType == Perspective)
	{
		aspect = aspectRatio;
		XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(fov, aspect, nearClip, farClip));
	}
	else if (projType == Orthographic)
	{
		XMStoreFloat4x4(&projMatrix, XMMatrixOrthographicLH(orthoWidth, orthoHeight, nearClip, farClip));
	}
}

void Camera::SetFov(float val)
{
	fov = val;
	UpdateProjectionMatrix(aspect);
}

void Camera::SetHeight(float val)
{
	orthoHeight = val;
	UpdateProjectionMatrix(aspect);
}

void Camera::SetWidth(float val)
{
	orthoWidth = val;
	UpdateProjectionMatrix(aspect);
}

void Camera::SetNearClip(float val)
{
	nearClip = val;
	UpdateProjectionMatrix(aspect);
}

void Camera::SetFarClip(float val)
{
	farClip = val;
	UpdateProjectionMatrix(aspect);
}

void Camera::SetProjectionType(ProjectionType val)
{
	projType = val;
	UpdateProjectionMatrix(aspect);
}

void Camera::Update(float dt)
{
	Input& input = Input::GetInstance();

#pragma region Keyboard Controls

	// Alter camera movement speeds
	float regMovSpeed = movSpeed;
	float regMouseSpeed = mouseSpeed;

	if (input.KeyDown(VK_SHIFT) && input.KeyUp(VK_CONTROL))
	{
		movSpeed *= 2.0f;
		mouseSpeed *= 2.0f;
	}
	else if (input.KeyRelease(VK_SHIFT))
	{
		movSpeed /= 2.0f;
		mouseSpeed /= 2.0f;
	}

	if (input.KeyDown(VK_CONTROL) && input.KeyUp(VK_SHIFT))
	{
		movSpeed *= 0.2f;
		mouseSpeed *= 0.2f;
	}
	else if (input.KeyRelease(VK_CONTROL))
	{
		movSpeed /= 0.2f;
		mouseSpeed /= 0.2f;
	}

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

		float pitch = transform.GetRotationPitchYawRoll().x;
		float yaw = transform.GetRotationPitchYawRoll().y;
		float roll = transform.GetRotationPitchYawRoll().z;

		if (cursorMovementY > 0)
		{
			pitch += mouseSpeed * dt * (float)cursorMovementY;

			// Clamp the rotation so the farthest down the camera can look is straight down
			if (pitch > Deg2Rad(90))
			{
				pitch = Deg2Rad(89.9f);
			}

			transform.SetRotation(pitch, yaw, roll);
		}
		else if (cursorMovementY < 0)
		{
			pitch += mouseSpeed * dt * (float)cursorMovementY;

			// Clamp the rotation so the farthest up the camera can look is straight up
			if (pitch < Deg2Rad(-90))
			{
				pitch = Deg2Rad(-89.9f);
			}

			transform.SetRotation(pitch, yaw, roll);
		}

		if (cursorMovementX > 0)
		{
			yaw += mouseSpeed * dt * (float)cursorMovementX;

			transform.SetRotation(pitch, yaw, roll);
		}
		else if (cursorMovementX < 0)
		{
			yaw += mouseSpeed * dt * (float)cursorMovementX;

			transform.SetRotation(pitch, yaw, roll);
		}
	}

#pragma endregion

	movSpeed = regMovSpeed;
	mouseSpeed = regMouseSpeed;

	UpdateViewMatrix();
}
