#pragma once

#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include <functional>

namespace UTextureUtils
{
	static bool BlitTextureToRenderTarget(const TObjectPtr<UTexture2D> source,
										  TObjectPtr<UTextureRenderTarget2D>& output,
										  FIntPoint subRect = FIntPoint::ZeroValue,
										  const std::function<void()>& onWriteComplete = nullptr);

	static bool BlitTextureToRenderTarget_Immediate(const TObjectPtr<UTexture2D> source,
													TObjectPtr<UTextureRenderTarget2D>& output,
													FIntPoint subRect = FIntPoint::ZeroValue);
};