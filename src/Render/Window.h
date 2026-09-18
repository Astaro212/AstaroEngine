#pragma once
#include <Windows.h>
#include <cstdint>

class Window {
public: 
	Window(uint32_t width, uint32_t height, const wchar_t* title);
	~Window();

	HWND GetHandle() const { return m_hwnd; }
	bool isOpen() const { return m_isOpen; }
	void ProcessMessages();

private: 
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	HWND m_hwnd;
	bool m_isOpen = true;
};