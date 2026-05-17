#pragma once

#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include <functional>
#include <vector>

class UTexture2D;
class UTextureRenderTarget2D;

namespace RenderUtils
{
	static bool BlitTextureToRenderTarget(const TObjectPtr<UTexture2D> source,
										  TObjectPtr<UTextureRenderTarget2D>& output,
										  FIntPoint subRect = FIntPoint::ZeroValue,
										  const std::function<void()>& onWriteComplete = nullptr);

	static bool BlitTextureToRenderTarget_Immediate(const TObjectPtr<UTexture2D> source,
													TObjectPtr<UTextureRenderTarget2D>& output,
													FIntPoint subRect = FIntPoint::ZeroValue);


	struct Mip
	{
		size_t mWidth = 0;
		size_t mHeight = 0;
		size_t mSlices = 0;
		uint8_t mChannels = 0;
		void* mPixels = nullptr;
	};

	static void GenerateMipsInt8(std::vector<Mip>& output,
								 uint8_t* const src,
								 size_t srcWidth, 
								 size_t srcHeight,
								 size_t srcLayers,
								 uint8_t srcChannels);

	static void GenerateMipsUInt32(std::vector<Mip>& output,
								   uint32_t* const src,
								   size_t srcWidth, 
								   size_t srcHeight,
								   size_t srcLayers,
								   uint8_t srcChannels);

	static void GenerateMipsInt32(std::vector<Mip>& output,
								  int32_t* const src,
								  size_t srcWidth, 
								  size_t srcHeight,
								  size_t srcLayers,
								  uint8_t srcChannels);

	static void GenerateMipsFloat16(std::vector<Mip>& output,
									FFloat16* const src,
									size_t srcWidth,
									size_t srcHeight,
									size_t srcLayers,
									uint8_t srcChannels);

	static void GenerateMipsFloat(std::vector<Mip>& output,
								  float* const src,
								  size_t srcWidth,
								  size_t srcHeight,
								  size_t srcLayers,
								  uint8_t srcChannels);
};