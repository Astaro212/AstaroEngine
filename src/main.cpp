#include "Render/DX12/DX12Core.h"
#include "Render/Window.h"
#include "Managers/ComponentManager.h"
#include "Systems/Input/InputSystem.h"
#include "Systems/Camera/CameraSystem.h"
#include "Render/Physics/PhysicsSystem.h"
#include "Render/RenderSystem.h"
#include <iostream>
#include <chrono>
#include <memory>

// Global variables
Window* g_Window = nullptr;
DX12Core* g_dx12 = nullptr;
ComponentManager* g_componentManager = nullptr;

// Systems
InputSystem* g_inputSystem = nullptr;
CameraSystem* g_cameraSystem = nullptr;
PhysicsSystem* g_physicsSystem = nullptr;
RenderSystem* g_renderSystem = nullptr;

void CreatePlayer() {
	// Создаём игрока
	EntityID player = g_componentManager->CreateEntity();
	
	// Добавляем компоненты
	/*g_componentManager->AddComponent<Position>(player, 0.0f, 1.0f, 0.0f);
	g_componentManager->AddComponent<Rotation>(player, 0.0f, 0.0f, 0.0f);
	g_componentManager->AddComponent<Velocity>(player, 0.0f, 0.0f, 0.0f);
	g_componentManager->AddComponent<Health>(player);
	g_componentManager->AddComponent<Mesh>(player);
	g_componentManager->AddComponent<Camera>(player);*/
	
	// Помечаем как игровую камеру
	/*auto* camera = g_componentManager->GetComponent<Camera>(player);
	camera->isActive = true;*/
}

int main(int argc, char* argv[]) {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "Starting MMORPG Engine..." << std::endl;

	try {
		// Инициализируем окно
		std::cout << "Creating window..." << std::endl;
		g_Window = new Window(1280, 720, L"Astaro Engine");

		if (!g_Window) {
			std::cout << "Failed to create window..." << std::endl;
			return -1;
		}

		// Инициализируем DX12
		std::cout << "Initializing DX12Core..." << std::endl;
		g_dx12 = new DX12Core();
		g_dx12->Init(g_Window->GetHandle());

		// Инициализируем ComponentManager
		g_componentManager = new ComponentManager();

		// Создаём системы
		std::cout << "Creating game systems..." << std::endl;
		//g_inputSystem = new InputSystem(g_componentManager, g_dx12);
		//g_cameraSystem = new CameraSystem(g_componentManager, g_dx12);
		//g_physicsSystem = new PhysicsSystem(g_componentManager, g_dx12);
		//g_renderSystem = new RenderSystem(g_componentManager, g_dx12);

		// Создаём игрока
		std::cout << "Creating player..." << std::endl;
		CreatePlayer();

		std::cout << "DX12Core initialized successfully..." << std::endl;
		std::cout << "Press ESC to exit window..." << std::endl;
		std::cout << "Use WASD to move, arrow keys for camera..." << std::endl;

		std::cout << "Entering render phase! Main loop started..." << std::endl;

		// Главный игровой цикл
		auto lastTime = std::chrono::high_resolution_clock::now();

		while (g_Window->isOpen()) {
			// Обработка сообщений окна
			g_Window->ProcessMessages();

			if (!g_Window->isOpen()) break;

			// Рассчитываем deltaTime
			auto currentTime = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float>(
				currentTime - lastTime
			).count();
			lastTime = currentTime;

			// Ограничиваем максимальный deltaTime
			if (deltaTime > 0.033f) deltaTime = 0.033f;

			// Обновляем системы в правильном порядке
			//g_inputSystem->Update(deltaTime);
			//g_physicsSystem->Update(deltaTime);
			//g_cameraSystem->Update(deltaTime);
			//g_renderSystem->Update(deltaTime);

			// Рендеринг
			g_dx12->Render();
		}

	} catch (const std::exception& ex) {
		std::cout << "CRITICAL ERROR: " << ex.what() << std::endl;
		MessageBoxA(nullptr, ex.what(), "Engine Error", MB_OK | MB_ICONERROR);
	}

	// Очистка
	std::cout << "Shutting down!" << std::endl;

	if (g_renderSystem) {
		delete g_renderSystem;
		g_renderSystem = nullptr;
	}
	if (g_cameraSystem) {
		delete g_cameraSystem;
		g_cameraSystem = nullptr;
	}
	if (g_physicsSystem) {
		delete g_physicsSystem;
		g_physicsSystem = nullptr;
	}
	if (g_inputSystem) {
		delete g_inputSystem;
		g_inputSystem = nullptr;
	}
	if (g_componentManager) {
		delete g_componentManager;
		g_componentManager = nullptr;
	}
	if (g_dx12) {
		g_dx12->Shutdown();
		delete g_dx12;
		g_dx12 = nullptr;
	}
	if (g_Window) {
		delete g_Window;
		g_Window = nullptr;
	}

	if (f) fclose(f);

	std::cout << "Engine shutdown complete!" << std::endl;
	return 0;
}