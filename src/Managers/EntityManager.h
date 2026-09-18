#pragma once
#include <typeinfo>
#include <cstdint>
#include <vector>
#include <bitset>
#include <memory>
#include "ComponentManager.h"
using Entity = size_t;

class EntityManager {
    // Маски для быстрой фильтрации (HasComponent)
    std::bitset<MAX_COMPONENTS> m_signatures[MAX_ENTITIES];
    uint32_t m_livingEntityCount = 0;

    // Внутреннее хранилище для типа T
    template<typename T>
    class ComponentPool {
    public:
        T m_data[MAX_ENTITIES];
        EntityID m_entityToIndex[MAX_ENTITIES];
        EntityID m_indexToEntity[MAX_ENTITIES];
        uint32_t m_size = 0;

        void Insert(EntityID entity, T component) {
            uint32_t index = m_size;
            m_entityToIndex[entity] = index;
            m_indexToEntity[index] = entity;
            m_data[index] = component;
            m_size++;
        }

        void Remove(EntityID entity) {
            // Swap-and-Pop: заменяем удаляемый элемент последним
            uint32_t removedIndex = m_entityToIndex[entity];
            uint32_t lastIndex = m_size - 1;
            m_data[removedIndex] = m_data[lastIndex];

            // Обновляем карты индексов
            EntityID lastEntity = m_indexToEntity[lastIndex];
            m_entityToIndex[lastEntity] = removedIndex;
            m_indexToEntity[removedIndex] = lastEntity;

            m_size--;
        }
    };

    // Массив указателей на пулы (по одному на тип)
    std::unique_ptr<void*, void(*)(void*)> m_pools[MAX_COMPONENTS];
    // (Для простоты в MVP можно юзать std::any или void*)

public:
    EntityID CreateEntity() { return m_livingEntityCount++; }

    template<typename T>
    void AddComponent(EntityID entity, T component) {
        // Логика добавления в пул по ComponentID<T>
        // И установка бита в m_signatures[entity]
    }
};
