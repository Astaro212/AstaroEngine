#pragma once
#include <memory>
#include <bitset>
#include <unordered_map>
#include <Windows.h>

using namespace std;

constexpr size_t MAX_COMPONENTS = 32;
using Signature = bitset<MAX_COMPONENTS>;

class System;

class SystemManager{

public:
	template<typename T> shared_ptr<T> registerSystem() {
		static_assert(is_base<System, T>::value, "T must be from System");
		const char* typeName = typeid(T).name();
		auto system = make_shared<T>();
		m_systems.insert({ typeName, system });
		return system;
	}

	template<typename T> void setSystemSignature(Signature sign) {
		auto typeName = typeid(T).name();
		m_signatures[type] = sign;
	}

private: 
	unordered_map<const char*, Signature> m_signatures;
	unordered_map<const char*, std::shared_ptr<System>> m_systems;
};

