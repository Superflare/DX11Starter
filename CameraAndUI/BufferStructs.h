#pragma once

#include <DirectXMath.h>

struct VertexShaderExternalData
{
	VertexShaderExternalData()
	{
		colorTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&viewMatrix, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&projMatrix, DirectX::XMMatrixIdentity());
	}

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;
	DirectX::XMFLOAT4 colorTint;
};