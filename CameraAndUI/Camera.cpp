#include "Camera.h"
#include "Input.h"
#include <cmath>
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
	projType(projType),
	pitch(0.0f),
	yaw(0.0f)
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

// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Quaternion_to_Euler_angles_conversion
// Used to update camera's internal pitch and yaw fields when it is rotated from outside of Update()
void Camera::SyncRotationWithTransform()
{
	XMFLOAT4 q = transform.GetRotation();

	// Rotation around x-axis
	float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1 - 2 * (q.x * q.x * q.y * q.y);
	pitch = atan2(sinr_cosp, cosr_cosp);

	// Rotation around y-axis
	float sinp = 2 * (q.w * q.y - q.z * q.x);
	if (abs(sinp) >= 1)
	{
		yaw = copysign(XM_PIDIV2, sinp);
	}
	else
	{
		yaw = asin(sinp);
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

	// Alter camera movement speeds
	float regMovSpeed = movSpeed;
	float regMouseSpeed = mouseSpeed;

	if (input.KeyDown(VK_SHIFT) && input.KeyUp(VK_CONTROL))
	{
		movSpeed *= 1.5f;
		mouseSpeed *= 1.5f;
	}
	else if (input.KeyRelease(VK_SHIFT))
	{
		movSpeed /= 1.5f;
		mouseSpeed /= 1.5f;
	}

	if (input.KeyDown(VK_CONTROL) && input.KeyUp(VK_SHIFT))
	{
		movSpeed *= 0.5f;
		mouseSpeed *= 0.5f;
	}
	else if (input.KeyRelease(VK_CONTROL))
	{
		movSpeed /= 0.5f;
		mouseSpeed /= 0.5f;
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

		if (cursorMovementY > 0)
		{
			pitch += mouseSpeed * dt;

			// Clamp the rotation so the farthest down the camera can look is straight down
			if (pitch > XM_PIDIV2)
			{
				pitch = XM_PIDIV2;
			}

			transform.SetRotation(pitch, yaw, 0);
		}
		else if (cursorMovementY < 0)
		{
			pitch -= mouseSpeed * dt;

			// Clamp the rotation so the farthest up the camera can look is straight up
			if (pitch < -XM_PIDIV2)
			{
				pitch = -XM_PIDIV2;
			}

			transform.SetRotation(pitch, yaw, 0);
		}

		if (cursorMovementX > 0)
		{
			yaw += mouseSpeed * dt;

			transform.SetRotation(pitch, yaw, 0);
		}
		else if (cursorMovementX < 0)
		{
			yaw -= mouseSpeed * dt;

			transform.SetRotation(pitch, yaw, 0);
		}
	}

#pragma endregion

	movSpeed = regMovSpeed;
	mouseSpeed = regMouseSpeed;

	UpdateViewMatrix();
}
