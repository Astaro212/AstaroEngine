#pragma once
#include "Systems/System.h"
#include <DirectXMath.h>

class CameraSystem : public System {
public:
	CameraSystem(ComponentManager* cm, DX12Core* renderer);
	void Update(float deltaTime) override;

	DirectX::XMFLOAT4X4 GetViewMatrix() const { return m_viewMatrix; }
	DirectX::XMFLOAT4X4 GetProjectionMatrix() const { return m_projectionMatrix; }

private:
	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_projectionMatrix;
	
	//EntityID m_playerEntity = 0;
	const float m_cameraDistance = 5.0f;
	const float m_cameraHeight = 3.0f;
};

