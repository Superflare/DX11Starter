#pragma once

#include <string>

// Helpers for determining the actual path to the executable
std::wstring GetExePath();
std::wstring FixPath(const std::wstring& relativeFilePath);
std::string WideToNarrow(const std::wstring& str);
std::wstring NarrowToWide(const std::string& str);
float Deg2Rad(float deg);
DirectX::XMFLOAT3 Deg2RadFromVector(DirectX::XMFLOAT3 degV);
float Rad2Deg(float rad);
DirectX::XMFLOAT3 Rad2DegFromVector(DirectX::XMFLOAT3 radV);