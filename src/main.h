#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>


using namespace std;

class EntityManager;
class ComponentManager;
class SystemManager;
class System;

using Entity = size_t;

class ECSCoordinator {

public:
	void init() {
		m_componentManager = make_unique<ComponentManager>();
		m_entityManager = make_unique<EntityManager>();
		m_systemManager = make_unique<SystemManager>();
	};
	Entity createEntity() {
		return m_entityManager->createEntity();
	};
	void destroyEntity(Entity entity) {
	};

	template<typename T> void registerComponent() {
		m_componentManager->template registerComponent<T>;
	};
	template<typename T> void addComponent(Entity entity, T component) {
		m_componentManager-> template addComponent<T>(entity, component);
	};

	template<typename T> T& getComponent(Entity entity) {
		return m_componentManager-> template getComponent(entity);
	};
	template<typename T> void registerSystem() {
		return m_systemManager->register<T>();
	};

private:
	unique_ptr<ComponentManager> m_componentManager;
	unique_ptr<EntityManager> m_entityManager;
	unique_ptr<SystemManager> m_systemManager;

};