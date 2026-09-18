#include "GuiComponents.h"

struct ButtonComponent {
	float x, y, width, height;
	const char* text;
	bool isPressed;
};

struct RenderableComponent {
	float color[4];
};
