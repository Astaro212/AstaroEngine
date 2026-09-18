#include "InputSystem.h"
#include "Managers/ComponentManager.h"

InputSystem::InputSystem(ComponentManager* cm, DX12Core* renderer)
	: System(cm, renderer) {
}

void InputSystem::Update(float deltaTime) {
	UpdateKeyboard();
	UpdateMouse();
}

void InputSystem::UpdateKeyboard() {
	// Горизонтальное движение (A/D или Left/Right)
	m_horizontalAxis = 0.0f;
	if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000) {
		m_horizontalAxis = -1.0f;
	}
	if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		m_horizontalAxis = 1.0f;
	}

	// Вертикальное движение (W/S или Up/Down)
	m_verticalAxis = 0.0f;
	if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000) {
		m_verticalAxis = 1.0f;
	}
	if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {
		m_verticalAxis = -1.0f;
	}

}

void InputSystem::UpdateMouse() {
	POINT mousePos;
	GetCursorPos(&mousePos);
	m_mouseX = mousePos.x;
	m_mouseY = mousePos.y;
}

bool InputSystem::IsKeyPressed(int key) const {
	return (GetAsyncKeyState(key) & 0x8000) != 0;
}