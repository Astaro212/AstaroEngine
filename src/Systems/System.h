#pragma once

class ComponentManager;
class DX12Core;

class System
{
public:
	virtual ~System() = default;
	virtual void Update(float deltaTime) = 0;
	
protected:
	ComponentManager* m_componentManager = nullptr;
	DX12Core* m_renderer = nullptr;

	System(ComponentManager* cm, DX12Core* renderer)
		: m_componentManager(cm), m_renderer(renderer) {
	}

};

