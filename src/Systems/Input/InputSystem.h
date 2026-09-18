#pragma once
#include "Systems/System.h"
#include <Windows.h>

class InputSystem : public System {
public:
	InputSystem(ComponentManager* cm, DX12Core* renderer);
	void Update(float deltaTime) override;

	// Input состояние
	bool IsKeyPressed(int key) const;
	float GetHorizontalInput() const { return m_horizontalAxis; }
	float GetVerticalInput() const { return m_verticalAxis; }
	float GetMouseX() const { return m_mouseX; }
	float GetMouseY() const { return m_mouseY; }

private:
	float m_horizontalAxis = 0.0f;  // A/D или Left/Right
	float m_verticalAxis = 0.0f;    // W/S или Up/Down
	int m_mouseX = 0;
	int m_mouseY = 0;

	void UpdateKeyboard();
	void UpdateMouse();
};
