#include "Material.h"

Material::Material(std::shared_ptr<SimpleVertexShader> vxShader, std::shared_ptr<SimplePixelShader> pxShader,
	DirectX::XMFLOAT4 colorTint, float roughness)
	:
	vertexShader(vxShader),
	pixelShader(pxShader),
	colorTint(colorTint),
	roughness(roughness)
{
}
