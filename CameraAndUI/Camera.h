#pragma once
#include "Transform.h"

class Camera
{
	enum ProjectionType {
		Perspective,
		Orthographic
	};

public:
	Camera(DirectX::XMFLOAT3 startPos, DirectX::XMFLOAT4 startRot,
		float aspect = 16.0f/9.0f, float fov = DirectX::XM_PIDIV4,
		float nearClip = 0.1f, float farClip = 1000.0f,
		float movSpeed = 10.0f, float mouseSpeed = 1.0f,
		ProjectionType projType = Perspective);

	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspect);

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	void Update(float dt);

private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	float fov;
	float nearClip;
	float farClip;
	float movSpeed;
	float mouseSpeed;
	ProjectionType projType;
};

