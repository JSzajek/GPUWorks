#pragma once

#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include <functional>
#include <vector>

class UTexture2D;
class UTextureRenderTarget2D;

namespace RenderUtils
{
	/// <summary>
	///	Blits a UTexture2D to a UTextureRenderTarget2D.
	///	This is an asynchronous operation, and the onWriteComplete callback will be called when the operation is complete.
	/// </summary>
	/// <param name="source">The source texture</param>
	/// <param name="output">The output render target</param>
	/// <param name="subRect">The region to copy</param>
	/// <param name="onWriteComplete">The on write complete callback</param>
	/// <returns>True if the operation was successful</returns>
	static bool BlitTextureToRenderTarget(const TObjectPtr<UTexture2D> source,
										  TObjectPtr<UTextureRenderTarget2D>& output,
										  FIntPoint subRect = FIntPoint::ZeroValue,
										  const std::function<void()>& onWriteComplete = nullptr);

	/// <summary>
	///	Blits a UTexture2D to a UTextureRenderTarget2D.
	/// </summary>
	/// <param name="source">The source texture</param>
	/// <param name="output">The output render target</param>
	/// <param name="subRect">The region to copy</param>
	/// <param name="onWriteComplete">The on write complete callback</param>
	/// <returns>True if the operation was successful</returns>
	static bool BlitTextureToRenderTarget_Immediate(const TObjectPtr<UTexture2D> source,
													TObjectPtr<UTextureRenderTarget2D>& output,
													FIntPoint subRect = FIntPoint::ZeroValue);


	/// <summary>
	/// Structure representing a single Mip level of a texture, 
	/// containing the dimensions, number of slices, 
	/// number of channels and a pointer to the pixel data.
	/// </summary>
	struct Mip
	{
		size_t mWidth = 0;
		size_t mHeight = 0;
		size_t mSlices = 0;
		uint8_t mChannels = 0;
		void* mPixels = nullptr;
	};

	/// <summary>
	/// Generates mipmaps for uint8_t data.
	/// The function takes the source pixel data and its dimensions,
	/// </summary>
	/// <param name="output">The output mip levels</param>
	/// <param name="src">The source pixel data</param>
	/// <param name="srcWidth">The width of the source texture</param>
	/// <param name="srcHeight">The height of the source texture</param>
	/// <param name="srcLayers">The number of layers in the source texture</param>
	/// <param name="srcChannels">The number of channels in the source texture</param>
	static void GenerateMipsUInt8(std::vector<Mip>& output,
								  uint8_t* const src,
								  size_t srcWidth, 
								  size_t srcHeight,
								  size_t srcLayers,
								  uint8_t srcChannels);

	/// <summary>
	/// Generates mipmaps for uint32_t data.
	/// The function takes the source pixel data and its dimensions.
	/// </summary>
	/// <param name="output">The output mip levels</param>
	/// <param name="src">The source pixel data</param>
	/// <param name="srcWidth">The width of the source texture</param>
	/// <param name="srcHeight">The height of the source texture</param>
	/// <param name="srcLayers">The number of layers in the source texture</param>
	/// <param name="srcChannels">The number of channels in the source texture</param>
	static void GenerateMipsUInt32(std::vector<Mip>& output,
								   uint32_t* const src,
								   size_t srcWidth, 
								   size_t srcHeight,
								   size_t srcLayers,
								   uint8_t srcChannels);

	/// <summary>
	/// Generates mipmaps for int32_t data.
	/// The function takes the source pixel data and its dimensions.
	/// </summary>
	/// <param name="output">The output mip levels</param>
	/// <param name="src">The source pixel data</param>
	/// <param name="srcWidth">The width of the source texture</param>
	/// <param name="srcHeight">The height of the source texture</param>
	/// <param name="srcLayers">The number of layers in the source texture</param>
	/// <param name="srcChannels">The number of channels in the source texture</param>
	static void GenerateMipsInt32(std::vector<Mip>& output,
								  int32_t* const src,
								  size_t srcWidth, 
								  size_t srcHeight,
								  size_t srcLayers,
								  uint8_t srcChannels);

	/// <summary>
	/// Generates mipmaps for FFloat16 data.
	/// The function takes the source pixel data and its dimensions.
	/// </summary>
	/// <param name="output">The output mip levels</param>
	/// <param name="src">The source pixel data</param>
	/// <param name="srcWidth">The width of the source texture</param>
	/// <param name="srcHeight">The height of the source texture</param>
	/// <param name="srcLayers">The number of layers in the source texture</param>
	/// <param name="srcChannels">The number of channels in the source texture</param>
	static void GenerateMipsFloat16(std::vector<Mip>& output,
									FFloat16* const src,
									size_t srcWidth,
									size_t srcHeight,
									size_t srcLayers,
									uint8_t srcChannels);

	/// <summary>
	/// Generates mipmaps for float data.
	/// The function takes the source pixel data and its dimensions.
	/// </summary>
	/// <param name="output">The output mip levels</param>
	/// <param name="src">The source pixel data</param>
	/// <param name="srcWidth">The width of the source texture</param>
	/// <param name="srcHeight">The height of the source texture</param>
	/// <param name="srcLayers">The number of layers in the source texture</param>
	/// <param name="srcChannels">The number of channels in the source texture</param>
	static void GenerateMipsFloat(std::vector<Mip>& output,
								  float* const src,
								  size_t srcWidth,
								  size_t srcHeight,
								  size_t srcLayers,
								  uint8_t srcChannels);
};