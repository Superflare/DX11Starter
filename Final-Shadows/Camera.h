#pragma once

#include "Transform.h"

class Camera
{
public:
	enum ProjectionType {
		Perspective,
		Orthographic
	};

	Camera(DirectX::XMFLOAT3 startPos, DirectX::XMFLOAT4 startRot,
		float aspect = 16.0f/9.0f, float fov = DirectX::XM_PIDIV4,
		float nearClip = 0.1f, float farClip = 1000.0f,
		float movSpeed = 10.0f, float mouseSpeed = 0.2f,
		ProjectionType projType = Perspective);

	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspect);

	DirectX::XMFLOAT4X4 GetViewMatrix() { return viewMatrix; }
	DirectX::XMFLOAT4X4 GetProjectionMatrix() { return projMatrix; }

	Transform* GetTransform() { return &transform; }
	float GetFov() { return fov; }
	float GetWidth() { return orthoWidth; }
	float GetHeight() { return orthoHeight; }
	float GetNearClip() { return nearClip; }
	float GetFarClip() { return farClip; }
	ProjectionType GetProjectionType() { return projType; }

	void SetFov(float val);
	void SetHeight(float val);
	void SetWidth(float val);
	void SetNearClip(float val);
	void SetFarClip(float val);
	void SetProjectionType(ProjectionType val);

	void Update(float dt);

private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	float aspect;
	float fov;
	float orthoWidth;
	float orthoHeight;
	float nearClip;
	float farClip;
	float movSpeed;
	float mouseSpeed;
	ProjectionType projType;
};
