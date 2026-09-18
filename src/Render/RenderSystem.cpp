#include "RenderSystem.h"
#include "Managers/ComponentManager.h"
#include "Render/DX12/DX12Core.h"
#include <DirectXMath.h>

RenderSystem::RenderSystem(ComponentManager* cm, DX12Core* renderer)
	: System(cm, renderer) {
}

void RenderSystem::Update(float deltaTime) {
	/*using namespace DirectX;
	
	// Получаем все сущности с Mesh компонентом
	auto renderables = m_componentManager->GetEntitiesWithComponent<Mesh>();
	
	for (EntityID id : renderables) {
		auto* pos = m_componentManager->GetComponent<Position>(id);
		auto* rot = m_componentManager->GetComponent<Rotation>(id);
		auto* mesh = m_componentManager->GetComponent<Mesh>(id);
		
		if (!pos || !rot || !mesh) continue;
		
		// Здесь отправляем данные на GPU для отрисовки
		// Пока просто логируем
		// (далее будет интеграция с DX12Core)
	}
}

DirectX::XMFLOAT4X4 RenderSystem::GetWorldMatrix(
	const Position* pos, 
	const Rotation* rot) const {
	
	using namespace DirectX;
	
	XMMATRIX world = XMMatrixMultiply(
		XMMatrixRotationRollPitchYaw(rot->pitch, rot->yaw, rot->roll),
		XMMatrixTranslation(pos->x, pos->y, pos->z)
	);
	
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, world);
	return result;*/
}
