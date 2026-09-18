#include "Render/Physics/PhysicsSystem.h"
#include "Managers/ComponentManager.h"
#include "Systems/Input/InputSystem.h"
#include <cmath>

PhysicsSystem::PhysicsSystem(ComponentManager* cm, DX12Core* renderer)
	: System(cm, renderer) {
}

void PhysicsSystem::Update(float deltaTime) {
	/*
	Получаем все сущности с компонентами Position и Velocity
	auto entities = m_componentManager->GetEntities;
	
	for (EntityID id : entities) {
		auto* pos = m_componentManager->GetComponent<Position>(id);
		auto* rot = m_componentManager->GetComponent<Rotation>(id);
		auto* vel = m_componentManager->GetComponent<Velocity>(id);
		
		if (!pos || !rot || !vel) continue;
		
		// Применяем гравитацию
		vel->vy -= m_gravity * deltaTime;
		
		// Обновляем позицию на основе скорости
		pos->x += vel->vx * deltaTime;
		pos->y += vel->vy * deltaTime;
		pos->z += vel->vz * deltaTime;
		
		// Простая коллизия с полом
		if (pos->y < 0.0f) {
			pos->y = 0.0f;
			vel->vy = 0.0f;
		}*/
	}