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
	}

	if (input.KeyDown(VK_CONTROL) && input.KeyUp(VK_SHIFT))
	{
		movSpeed *= 0.2f;
		mouseSpeed *= 0.2f;
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
	if (input.KeyDown('E') || input.KeyDown(VK_SPACE))
	{
		transform.MoveRelative(0, movSpeed * dt, 0);
	}
	if (input.KeyDown('Q'))
	{
		transform.MoveRelative(0, -movSpeed * dt, 0);
	}

#pragma endregion

#pragma region Mouse Controls

	if (input.MouseRightDown())
	{
		int cursorMovementX = input.GetMouseXDelta();
		int cursorMovementY = input.GetMouseYDelta();

		float prevPitch = transform.GetRotationPitchYawRoll().x;
		float pitch = 0;

		pitch = Deg2Rad(mouseSpeed * dt * (float)cursorMovementY);

		// Clamp the pitch rotation so the camera can't flip over and render upside down
		if (prevPitch + pitch > Deg2Rad(89.8f) || prevPitch + pitch < Deg2Rad(-89.8f))
			pitch = 0;

		float yaw = Deg2Rad(mouseSpeed * dt * (float)cursorMovementX);

		//transform.Rotate(pitch, yaw, 0);

		transform.RotateAxisLocal(pitch, XMFLOAT3(1, 0, 0));
		transform.RotateAxisWorld(yaw, XMFLOAT3(0, 1, 0));
	}

#pragma endregion

	movSpeed = regMovSpeed;
	mouseSpeed = regMouseSpeed;

	UpdateViewMatrix();
}
