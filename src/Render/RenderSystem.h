#pragma once
#include "Systems/System.h"
#include <DirectXMath.h>

class RenderSystem : public System {
public:
	RenderSystem(ComponentManager* cm, DX12Core* renderer);
	void Update(float deltaTime) override;

private:
	//DirectX::XMFLOAT4X4 GetWorldMatrix(const Position* pos, const Rotation* rot);
};