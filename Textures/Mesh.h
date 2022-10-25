#pragma once

#include <d3d11.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	Mesh(const wchar_t* objFile, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return vertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() {return indexBuffer; }
	int GetIndexCount() { return indexCount; }

	void Draw();

private:
	void CreateVertexIndexBuffers(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device);

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
};