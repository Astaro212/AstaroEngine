#include "Window.h"

Window::Window(uint32_t width, uint32_t height, const wchar_t* title) {
	WNDCLASS  wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = L"DX12WindowClass";
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

	RegisterClass(&wc);

	RECT rect = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	m_hwnd = CreateWindow(
		L"DX12WindowClass",
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		this  // Sending this for getting access to WndProc
	);

	if (m_hwnd) {
		ShowWindow(m_hwnd, SW_SHOW);
		UpdateWindow(m_hwnd);
	}
}

Window::~Window() {
	if (m_hwnd) {
		DestroyWindow(m_hwnd);
	}
}

void Window::ProcessMessages() {
	MSG msg = {};

	// Get all messages in queue
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			m_isOpen = false;
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Window* pWindow = nullptr;

	if (msg == WM_NCCREATE) {
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
	} else {
		pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (pWindow) {
		switch (msg) {
		case WM_DESTROY:
			pWindow->m_isOpen = false;
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				pWindow->m_isOpen = false;
				PostQuitMessage(0);
			}
			return 0;

		case WM_CLOSE:
			pWindow->m_isOpen = false;
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}