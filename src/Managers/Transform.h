#pragma once
#include <cstdint>
#include <bitset>
#include <DirectXMath.h>
struct Transform {
	DirectX::XMFLOAT4X4 matrix;
	Transform() {
		DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixIdentity());
	}
};

struct Mesh {
	std::string meshFile;
	std::string textureFile;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
};

struct Camera {
	float fov = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;
	bool isActive = false;
};