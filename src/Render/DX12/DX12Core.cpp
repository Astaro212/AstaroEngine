#include "DX12Core.h"
#include <chrono>
#include "Utils/Helper.h"
#include "Render/Textures/TextureLoader.h"
#include <iostream>

using namespace DirectX;


void DX12Core::Init(HWND hwnd) {
	EnableDebugLayer();

	// Window size
	RECT rect;
	GetClientRect(hwnd, &rect);

	m_windowWidth = rect.right - rect.left;
	m_windowHeight = rect.bottom - rect.top;


	//Create Factory
	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	// Create device (getting adapter)
	ThrowIfFailed(D3D12CreateDevice(
		nullptr, // Adapter (default)
		D3D_FEATURE_LEVEL_11_0, // Minimum feature (DirectX11) 
		IID_PPV_ARGS(&m_device)) // Get this device
	);

	D3D12_COMMAND_QUEUE_DESC queueDesc = {}; // Command Queue
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;  // Direct render commands
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cmdQueue)));


	CreateSwapChain(hwnd, m_windowWidth, m_windowHeight, factory.Get());

	CreateRenderTargets();

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 16; // Number of textures
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

	m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	CreateCommandObjects();

	CreateFence();

	CreatePipelineState();

	CreateUIVertexBuffer();

	CreateUIElements();

	std::cout << "DX12 initialized successfully!" << std::endl;
};

void DX12Core::CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory4* factory) {
	m_swapChain.Reset();

	// Swap chain settings
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_frameCount; // Buffer settings (3)
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // RGBA color format
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Use buffer for rendering things
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // rotate and forget
	swapChainDesc.SampleDesc.Count = 1; // Multisampling off

	// Creating swap chain (buffers)
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_cmdQueue.Get(),  // Command queue
		hwnd,  // Window
		&swapChainDesc,  //Settings
		nullptr,  // Fullscreen
		nullptr,  // Output?
		&swapChain  // Swap Chain
	));

	// Working with frame index
	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DX12Core::CreateRenderTargets() {

	// Heap desc
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = m_frameCount; // 1 desc for 1 buffer
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // Type: Render Target View
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	// Remembers descriptor size
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create render targets for every buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	m_renderTargets.resize(m_frameCount);

	for (UINT i = 0; i < m_frameCount; i++) {
		// Getting swap chain buffer
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));

		// Creating render target view (resource, description, descriptor)
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

		// Going to next descriptor
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
}

void DX12Core::CreateCommandObjects() {

	m_cmdAllocators.resize(m_frameCount);

	// Creating allocators (it's a memory for commands saving)
	for (UINT i = 0; i < m_frameCount; i++) {
		ThrowIfFailed(m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_cmdAllocators[i])
		));
	}

	// List of commands (this is instruction for GPU to do)
	ThrowIfFailed(
		m_device->CreateCommandList(
			0, // Node mask for multi-GPU (not needed by us)
			D3D12_COMMAND_LIST_TYPE_DIRECT, // Direct instructions
			m_cmdAllocators[0].Get(), // Getting command storage
			nullptr, // PSO (we don't have it's now
			IID_PPV_ARGS(&m_cmdList) // Getting list of commands to our memory
		)
	);

	ThrowIfFailed(m_cmdList->Close()); // Close list (it's open for new commands before)
}

void DX12Core::CreateFence() {
	// Init fence values
	m_fenceValues.resize(m_frameCount, 0);
	for (UINT i = 0; i < m_frameCount; i++) {
		m_fenceValues[i] = 0;
	}
	// Sync instructions
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))); // When we call IID_PPV_ARGS -> we put link & to variable
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);


	if (m_fenceEvent == nullptr) {
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	m_fenceValue = 1;
}

void DX12Core::CreatePipelineState() {
	// Creating pipeline
	// Here will be shaders, normals, heightmap, UV, etc
	CD3DX12_DESCRIPTOR_RANGE1 descRange;
	descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
	rootParameters[0].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_STATIC_SAMPLER_DESC sampler(0);
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;


	// Creating small signature
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignDesc;
	rootSignDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignDesc, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	// Вершинный шейдер
	const char* vsCode = R"(
        struct VS_INPUT {
            float3 pos : POSITION;
            float2 uv : TEXCOORD;
        };
        struct VS_OUTPUT {
            float4 pos : SV_POSITION;
            float2 uv : TEXCOORD;
        };
        VS_OUTPUT main(VS_INPUT input) {
            VS_OUTPUT output;
            output.pos = float4(input.pos, 1.0f);
            output.uv = input.uv;
            return output;
        }
    )";

	// Пиксельный шейдер
	const char* psCode = R"(
        Texture2D texture0 : register(t0);
        SamplerState sampler0 : register(s0);
        struct PS_INPUT {
            float4 pos : SV_POSITION;
            float2 uv : TEXCOORD;
        };
        float4 main(PS_INPUT input) : SV_TARGET {
            return texture0.Sample(sampler0, input.uv);
        }
    )";

	//  КОМПИЛИРУЕМ ОДИН РАЗ С ПРОВЕРКОЙ
	HRESULT hr = D3DCompile(
		vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
		"main", "vs_5_0", compileFlags, 0, &vertexShader, &error
	);

	if (FAILED(hr)) {
		if (error) {
			std::cout << "VERTEX SHADER ERROR: " << (char*)error->GetBufferPointer() << std::endl;
		}
		ThrowIfFailed(hr);
	}
	else {
		std::cout << " Vertex shader compiled successfully!" << std::endl;
	}

	hr = D3DCompile(
		psCode, strlen(psCode), nullptr, nullptr, nullptr,
		"main", "ps_5_0", compileFlags, 0, &pixelShader, &error
	);

	if (FAILED(hr)) {
		if (error) {
			std::cout << "PIXEL SHADER ERROR: " << (char*)error->GetBufferPointer() << std::endl;
		}
		ThrowIfFailed(hr);
	}
	else {
		std::cout << " Pixel shader compiled successfully!" << std::endl;
	}

	// Create PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// Root signature
	psoDesc.pRootSignature = m_rootSignature.Get();

	// Shaders
	psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

	// Rasterizer state
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Blending mode
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	// Depth (off for 2d)
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	// Formats
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	// Input layout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };

	// Create PSO
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	if (!m_pipelineState) {
		throw std::runtime_error("Failed to create Pipeline State Object");
	}
	else {
		std::cout << " Pipeline State created successfully!" << std::endl;
	}
}

void DX12Core::Render() {

	if (m_windowWidth == 0 || m_windowHeight == 0) {
		return; // Не рендерим если окно свернуто
	}
	// Waiting for previuos frame ends
	WaitForFrame(m_frameIndex);

	// Prepare!!!
	// Clear Allocator
	ThrowIfFailed(m_cmdAllocators[m_frameIndex]->Reset());


	// Clear ALL COMMANDS
	// Using CLEAR PSO (NOT USE IF A LOT OF OBJECTS)
	ThrowIfFailed(m_cmdList->Reset(m_cmdAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

	// Adding barriers (DX12 Specific)
	// 1 Barrier: Present->RenderTarget
	CD3DX12_RESOURCE_BARRIER barrierBefore = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, //Base
		D3D12_RESOURCE_STATE_RENDER_TARGET //Targer
	);

	m_cmdList->ResourceBarrier(1, &barrierBefore);

	// RENDER TARGET SETTINGS
	// Show render -> how and where to drow
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescriptorSize
	);

	m_cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Clear Screen (Color: blue)
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	RenderUI();


	// 2 Barrier: RenderTarget->Present
	CD3DX12_RESOURCE_BARRIER barrierAfter = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,    // Base
		D3D12_RESOURCE_STATE_PRESENT           // Target
	);
	m_cmdList->ResourceBarrier(1, &barrierAfter);

	// Finished commands
	ThrowIfFailed(m_cmdList->Close());

	// Send commands to GPU
	ID3D12CommandList* ppCommandLists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//Show what happened on screen
	ThrowIfFailed(m_swapChain->Present(1, 0));

	// Sync update
	const UINT64 currentFenceValue = m_fenceValue;
	ThrowIfFailed(m_cmdQueue->Signal(m_Fence.Get(), currentFenceValue));


	// Saving current buffer
	m_fenceValues[m_frameIndex] = currentFenceValue;
	m_fenceValue++;

	// Next buffer
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DX12Core::WaitForFrame(UINT frameIndex) {

	if (m_Fence->GetCompletedValue() < m_fenceValues[frameIndex]) {
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

}

void DX12Core::WaitForGPU() {
	const UINT64 fenceToWaitFor = m_fenceValue;
	ThrowIfFailed(m_cmdQueue->Signal(m_Fence.Get(), fenceToWaitFor));
	m_fenceValue++;

	if (m_Fence->GetCompletedValue() < fenceToWaitFor) {
		ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void DX12Core::Shutdown() {
	// GPU Waiting
	for (UINT i = 0; i < m_frameCount; i++) {
		WaitForFrame(i);
	}

	// Close all handlers
	if (m_fenceEvent) {
		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}
}

bool DX12Core::LoadTexture(const std::string& name, const wchar_t* filename) {
	std::vector<uint8_t> textureData;
	uint32_t width, height;

	if (!TextureLoader::LoadFromFile(filename, textureData, width, height)) {
		width = 256; height = 256;
		textureData.resize(width * height * 4);
		for (UINT y = 0; y < height; y++) {
			for (UINT x = 0; x < width; x++) {
				UINT idx = (y * width + x) * 4;
				bool isRed = ((x / 32) + (y / 32)) % 2 == 0;
				textureData[idx + 0] = isRed ? 255 : 0;
				textureData[idx + 1] = isRed ? 0 : 255;
				textureData[idx + 2] = 0;
				textureData[idx + 3] = 255;
			}
		}
	}


	// Creating GPU resources
	D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

	Texture texture;
	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture.resource)));

	texture.width = width;
	texture.height = height;

	// Copy to gpu
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.resource.Get(), 0, 1);
	ComPtr<ID3D12Resource> uploadBuffer;
	D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

	// Copy data
	D3D12_SUBRESOURCE_DATA textureSubresource = {};
	textureSubresource.pData = textureData.data();
	textureSubresource.RowPitch = width * 4;
	textureSubresource.SlicePitch = textureSubresource.RowPitch * height;

	//Command list for copy
	ComPtr<ID3D12CommandAllocator> uploadAllocator;
	ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCmdList)));

	UpdateSubresources(uploadCmdList.Get(), texture.resource.Get(), uploadBuffer.Get(), 0, 0, 1, &textureSubresource);

	//Barrier
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		texture.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	uploadCmdList->ResourceBarrier(1, &barrier);

	uploadCmdList->Close();

	// Do commands
	ID3D12CommandList* ppCommandLists[] = { uploadCmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	WaitForGPU();

	// Create SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	UINT descriptorOffset = static_cast<UINT>(m_textures.size()) * m_srvDescriptorSize;

	texture.cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	texture.cpuHandle.ptr += descriptorOffset;

	texture.gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	texture.gpuHandle.ptr += descriptorOffset;

	m_device->CreateShaderResourceView(texture.resource.Get(), &srvDesc, texture.cpuHandle);

	texture.width = width;
	texture.height = height;
	m_textures[name] = texture;

	return true;
}

void DX12Core::CreateUIElements() {
	if (LoadTexture("background", L"resources/background.jpg")) {
		std::cout << "Texture loaded successfully!" << std::endl;
		std::cout << "Texture count: " << m_textures.size() << std::endl;
		std::cout << "Texture dimensions: " << m_textures["background"].width << "x" << m_textures["background"].height << std::endl; 

		m_uiElements.push_back({ "background", 0, 0, 1920, 1080, false });
		std::cout << "UI elements count: " << m_uiElements.size() << std::endl;
	}
	else {
		std::cout << "Failed to load texture!" << std::endl;
	}
}

void DX12Core::RenderUI() {
	// Prepare pipeline state
	m_cmdList->SetPipelineState(m_pipelineState.Get());
	m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

	// Texture heap
	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	m_cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(m_windowWidth);    // Ширина окна
	viewport.Height = static_cast<float>(m_windowHeight);   // Высота окна  
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_cmdList->RSSetViewports(1, &viewport);

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = m_windowWidth;    // Ширина окна
	scissorRect.bottom = m_windowHeight;   // Высота окна
	m_cmdList->RSSetScissorRects(1, &scissorRect);


	m_cmdList->IASetVertexBuffers(0, 1, &m_uiVertexBufferView);
	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	for (const auto& element : m_uiElements) {
		auto it = m_textures.find(element.textureName);
		if (it != m_textures.end()) {
			// Set texture
			m_cmdList->SetGraphicsRootDescriptorTable(0, it->second.gpuHandle);

			// Setting transform
			// Draw quad
			m_cmdList->DrawInstanced(4, 1, 0, 0); //  4 vertices
		} else {
			std::cout << "Texture NOT found: " << element.textureName << std::endl;
		}
	}
}

void DX12Core::CreateUIVertexBuffer() {
	// Вершины для полноэкранного квада
	Vertex vertices[] = {
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) }, // нижний левый
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) }, // верхний левый  
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }, // нижний правый
		{ DirectX::XMFLOAT3(1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }  // верхний правый
	};

	const UINT vertexBufferSize = sizeof(vertices);

	// Создаем буфер в GPU памяти
	D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_uiVertexBuffer)));

	// Копируем данные вершин в буфер
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_uiVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, vertices, sizeof(vertices));
	m_uiVertexBuffer->Unmap(0, nullptr);

	// Создаем представление вершинного буфера
	m_uiVertexBufferView.BufferLocation = m_uiVertexBuffer->GetGPUVirtualAddress();
	m_uiVertexBufferView.StrideInBytes = sizeof(Vertex);
	m_uiVertexBufferView.SizeInBytes = vertexBufferSize;
}

void DX12Core::UpdateWindowSize(uint32_t width, uint32_t height) {
	if (width > 0 && height > 0) {
		m_windowWidth = width;
		m_windowHeight = height;
		std::cout << "Window size updated to: " << width << "x" << height << std::endl;
	}
}