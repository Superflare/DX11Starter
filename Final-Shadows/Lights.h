#pragma once

#include <DirectXMath.h>

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light {
	int type;						// directional, point, spot
	DirectX::XMFLOAT3 direction;
	float range;					// attenuation range
	DirectX::XMFLOAT3 position;
	float intensity;
	DirectX::XMFLOAT3 color;
	float spotFalloff;				// spot light cone size
	int castsShadows;				// 0 or 1 which indicated whether the light should cast shadows
	DirectX::XMFLOAT2 padding;		// purposeful padding to hit the 16-byte boundary
};