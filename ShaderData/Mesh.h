#pragma once

#include <d3d11.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();

	void Draw();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
};