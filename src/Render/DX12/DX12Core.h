#pragma once
#include <initguid.h>
#include <d3d12.h>
#include <directx/d3dx12.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <cstdint>
#include <wrl/client.h>
#include <wrl.h>
#include "Utils/Helper.h"
#include <vector>
#include <unordered_map>
#include <functional>



#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#if defined(CreateWindow)
#undef CreateWindow
#endif

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

struct UIElement {
	std::string textureName;
	float x, y, width, height;
	bool isButton = false;
	std::function<void()> onClick;
};

struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
};

class DX12Core {
public:
	void Init(HWND hwnd);
	void EnableDebugLayer() {
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugInterface;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
#endif
	};
	void Render();
	void Shutdown();
	void UpdateWindowSize(uint32_t width, uint32_t height);

private:
	struct Texture {
		ComPtr<ID3D12Resource> resource;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		UINT width, height;
	};

	std::unordered_map<std::string, Texture> m_textures;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescriptorSize;

	// Ui Elements
	

	std::vector<UIElement> m_uiElements;

	bool LoadTexture(const std::string& name, const wchar_t* filename);
	void CreateUIElements();
	void RenderUI();

	void CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory4* factory);
	void CreateRenderTargets();
	void CreateCommandObjects();
	void CreateFence();
	void CreatePipelineState();
	void WaitForFrame(UINT frameIndex);
	void WaitForGPU();
	void CreateUIVertexBuffer();

	uint32_t m_windowWidth;
	uint32_t m_windowHeight;
	
	//Main components
	ComPtr<ID3D12Device> m_device; //Videoadapter
	ComPtr<IDXGISwapChain4> m_swapChain; //Swap Chain
	ComPtr<ID3D12CommandQueue> m_cmdQueue; //Command queue

	//Render targets
	std::vector<ComPtr<ID3D12Resource>> m_renderTargets; //Render targets
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap; //Buffer heap descriptors
	UINT m_rtvDescriptorSize; //Size of descriptor

	// Commands
	std::vector<ComPtr<ID3D12CommandAllocator>> m_cmdAllocators; // Notepad for commands
	ComPtr<ID3D12GraphicsCommandList> m_cmdList; // Command list to execute

	// Sync
	ComPtr<ID3D12Fence> m_Fence; // Sync fence
	HANDLE m_fenceEvent;  // Fence event to sync
	UINT64 m_fenceValue; // Current fence

	// Pipeline
	ComPtr<ID3D12PipelineState> m_pipelineState; // Pipeline state
	ComPtr<ID3D12RootSignature> m_rootSignature; // Root signature 

	UINT m_frameIndex;
	static const UINT m_frameCount = 3;
	std::vector<UINT64> m_fenceValues;

	ComPtr<ID3D12Resource> m_uiVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_uiVertexBufferView;
	

public:
	void LoadBackgroundImage(const wchar_t* filename);
	void AddButton(float x, float y, float w, float h, const wchar_t* textureFile, std::function<void()> onClick);
};

