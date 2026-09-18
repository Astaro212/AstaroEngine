#include "CameraSystem.h"
#include "Managers/ComponentManager.h"
#include <DirectXMath.h>

CameraSystem::CameraSystem(ComponentManager* cm, DX12Core* renderer)
	: System(cm, renderer) {
	using namespace DirectX;
	
	// Инициализируем матрицы
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_projectionMatrix, XMMatrixPerspectiveFovLH(
		XM_PIDIV4,    // FOV 45 градусов
		1280.0f / 720.0f,  // Aspect ratio
		0.1f,         // Near plane
		1000.0f       // Far plane
	));
}

void CameraSystem::Update(float deltaTime) {
	/*using namespace DirectX;
	
	// Находим первую сущность с компонентом Camera
	auto cameras = m_componentManager->GetEntitiesWithComponent<Camera>();
	if (cameras.empty()) return;
	
	m_playerEntity = cameras[0];
	
	// Получаем позицию и ротацию игрока
	auto* pos = m_componentManager->GetComponent<Position>(m_playerEntity);
	auto* rot = m_componentManager->GetComponent<Rotation>(m_playerEntity);
	
	if (!pos || !rot) return;
	
	// Позиция камеры за спиной персонажа
	float px = pos->x + cosf(rot->yaw) * m_cameraDistance;
	float py = pos->y + m_cameraHeight;
	float pz = pos->z + sinf(rot->yaw) * m_cameraDistance;
	
	// Смотрим на персонажа
	XMVECTOR cameraPos = XMVectorSet(px, py, pz, 1.0f);
	XMVECTOR lookAt = XMVectorSet(pos->x, pos->y + 1.0f, pos->z, 1.0f);
	XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
	// Создаём матрицу вида
	XMMATRIX viewMatrix = XMMatrixLookAtLH(cameraPos, lookAt, upVector);
	XMStoreFloat4x4(&m_viewMatrix, viewMatrix);*/
}
