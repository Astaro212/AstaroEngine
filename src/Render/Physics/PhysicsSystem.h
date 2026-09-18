#pragma once
#include "Systems/System.h"

class PhysicsSystem : public System {
public:
	PhysicsSystem(ComponentManager* cm, DX12Core* renderer);
	void Update(float deltaTime) override;

private:
	const float m_gravity = 9.81f;
	const float m_moveSpeed = 5.0f;
	const float m_rotationSpeed = 3.0f;
};
