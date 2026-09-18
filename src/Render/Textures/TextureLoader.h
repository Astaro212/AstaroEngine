#pragma once
#include <wincodec.h>
#include <vector>

class TextureLoader
{
public:
	static bool LoadFromFile(const wchar_t* filename, std::vector<uint8_t>& textureData, uint32_t& width, uint32_t& height);
};

