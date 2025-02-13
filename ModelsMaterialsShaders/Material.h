#pragma once

#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"

class Material
{
public:
	Material(std::shared_ptr<SimpleVertexShader> vxShader, std::shared_ptr<SimplePixelShader> pxShader,
		DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(1,1,1,1));

	std::shared_ptr<SimpleVertexShader> GetVertexShader() { return vertexShader; }
	std::shared_ptr<SimplePixelShader> GetPixelShader() { return pixelShader; }
	DirectX::XMFLOAT4 GetColorTint() { return colorTint; }

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vxShader) { vertexShader = vxShader; }
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pxShader) { pixelShader = pxShader; }
	void SetColorTint(DirectX::XMFLOAT4 color) { colorTint = color; }

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};

