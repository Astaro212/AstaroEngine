#pragma once
#include <typeinfo>
#include <cstdint>
#include <vector>
#include <bitset>
#include <memory>

using ComponentID = uint32_t;
using EntityID = uint32_t;

const uint32_t MAX_ENTITIES = 10000;
const uint32_t MAX_COMPONENTS = 64;


namespace Internal {
	inline ComponentID GetNextComponentID() {
		static ComponentID lastID = 0;
		return lastID++;
	}
};

template<typename T>
inline ComponentID GetComponentTypeID() {
	static ComponentID typeID = Internal::GetNextComponentID();
	return typeID;
};


struct Position {
	float x, y, z;
	Position(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Rotation {
	float pitch, yaw, roll;
	Rotation(float p = 0, float y = 0, float r = 0) : pitch(p), yaw(y), roll(r) {}
};

struct Velocity {
	float vx, vy, vz;
	Velocity(float vx = 0, float vy = 0, float vz = 0) : vx(vx), vy(vy), vz(vz) {}
};

struct Health {
	float maxHealth = 100.0f;
	float currentHealth = 100.0f;
	bool isDead() const { return currentHealth <= 0; }
};

struct NetworkID {
	uint64_t id;
	bool isLocalPlayer = false;
	uint64_t ownerID = 0;
};

struct Inventory {
	std::vector<std::string> items;
	uint32_t gold = 0;
	void addItem(const std::string& item) { items.push_back(item); }
	bool hasItem(const std::string& item) const {
		return std::find(items.begin(), items.end(), item) != items.end();
	}
};

struct WorldContext {
	float globalTime = 0.0f;
	float entropy = 0.0f;
};

class ComponentManager {

	struct ComponentArrayBase {
		virtual ~ComponentArrayBase() = default;
		virtual void EntityDestroyed(EntityID entity = 0);
	};

	template<typename T>
	class ComponentArray : public ComponentArrayBase {
		std::vector<T> m_data;
		std::vector<size_t> m_entityToIndex;
		std::vector<EntityID> m_indexToEntity;
		size_t m_size = 0;

	public:
		ComponentArray() {
			m_data.resize(MAX_ENTITIES);
			m_entityToIndex.resize(MAX_ENTITIES, 0);
			m_indexToEntity.resize(MAX_ENTITIES, 0);
		}

		void InsertData(EntityID entity, T component) {
			size_t newIndex = m_size;
			m_entityToIndex[entity] = newIndex;
			m_indexToEntity[newIndex] = entity;
			m_data[newIndex] = component;
			m_size++;
		};

		T& GetData(EntityID entity) {
			return m_data[m_entityToIndex[entity]];
		};

		void RemoveData(EntityID entity) {
			// 1. Получаем индекс удаляемого компонента
			size_t indexOfRemovedEntity = m_entityToIndex[entity];

			// 2. Получаем индекс последнего активного компонента в массиве
			size_t indexOfLastElement = m_size - 1;

			// 3. Переносим данные последнего элемента на место удаляемого (Swap)
			m_data[indexOfRemovedEntity] = std::move(m_data[indexOfLastElement]);

			// 4. Обновляем маппинг для элемента, который мы только что переместили
			EntityID entityOfLastElement = m_indexToEntity[indexOfLastElement];
			m_entityToIndex[entityOfLastElement] = indexOfRemovedEntity;
			m_indexToEntity[indexOfRemovedEntity] = entityOfLastElement;

			// 5. Сбрасываем маппинг для удаленной сущности
			m_entityToIndex[entity] = 0;
			m_indexToEntity[indexOfLastElement] = 0;

			// 6. Уменьшаем размер логического массива (Pop)
			m_size--;
		}

	};

	std::vector<std::bitset<MAX_COMPONENTS>> m_entityMasks;

	std::vector<std::shared_ptr<ComponentArrayBase>> m_componentArrays;

public:
	ComponentManager() {
		m_entityMasks.resize(MAX_ENTITIES);
		m_componentArrays.resize(MAX_COMPONENTS);
	}


	EntityID CreateEntity() {
		return m_nextEntityID++;
	}

	template<typename T>
	void AddComponent(EntityID entity, T component) {
		ComponentID ct = GetComponentTypeID<T>();
		if (!m_componentArrays[ct]) {
			m_componentArrays[ct] = std::make_shared<ComponentArray<T>>();
		}

		static_cast<ComponentArray<T>*>(m_componentArrays[ct].get())->InsertData(entity, component);
		m_entityMasks[entity].set(ct);
	}

	template<typename T>
	T& GetComponent(EntityID entity) {
		ComponentID ct = GetComponentTypeID<T>();
		return static_cast<ComponentArray<T>*>(m_componentArrays[ct].get())->GetData(entity);
	}

	template<typename T>
	bool HasComponent(EntityID entity) {
		return m_entityMasks[entity].test(GetComponentTypeID<T>());
	}

	template<typename T>
	void RemoveComponent(EntityID entity) {
		ComponentID ct = GetComponentTypeID<T>();
		m_entityMasks[entity].reset(ct);
		return static_cast<ComponentArray<T>*>(m_componentArrays[ct].get())->RemoveData(entity);
	}

private:
	EntityID m_nextEntityID = 0;
};

