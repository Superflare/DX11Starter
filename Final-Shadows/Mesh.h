#pragma once

#include <d3d11.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <string>
#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context);
	Mesh(const wchar_t* objFile, const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context);
	Mesh(std::string objFile, const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return vertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() {return indexBuffer; }
	int GetIndexCount() { return indexCount; }

	void Draw();

private:
	void CreateVertexIndexBuffers(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount,
		const Microsoft::WRL::ComPtr<ID3D11Device>& device);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
};