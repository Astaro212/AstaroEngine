#include "TextureLoader.h"
#include <wincodec.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

bool TextureLoader::LoadFromFile(const wchar_t* filename, std::vector<uint8_t>& textureData, uint32_t& width, uint32_t& height) {


	// Init WIC
	ComPtr<IWICImagingFactory> wicFactory;
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&wicFactory));

	// Getting Image
	ComPtr<IWICBitmapDecoder> decoder;
	wicFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &decoder);

	// Getting first frame
	ComPtr<IWICBitmapFrameDecode> frame;
	decoder->GetFrame(0, &frame);

	// Getting image size
	frame->GetSize(&width, &height);

	// Convert to RGBA
	ComPtr<IWICFormatConverter> converter;
	wicFactory->CreateFormatConverter(&converter);
	converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
		WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

	// Copy data of image
	textureData.resize(width * height * 4);
	converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(textureData.size()), textureData.data());

	return true;

}
