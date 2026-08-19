#include "Interops/UE/GPUImageObject.h"
#include "Interops/UE/GPUContextObject.h"

#include "Engine/Texture.h"

#include "GPU/GPUContext.h"
#include "GPU/GPUQueue.h"
#include "GPU/GPUEvent.h"

struct FGPUTexture2DAsyncUpdate
{
	TWeakObjectPtr<UTexture2D> mpTexture;

	std::vector<uint8> mBaseBytes;
	std::vector<RenderUtils::Mip> mMips;
	std::vector<FUpdateTextureRegion2D> mRegions;

	std::vector<uint32> mSrcPitches;
	std::vector<uint32> mSrcBpps;

	std::function<void(bool)> mCompletionCallback;
};

bool UGPUImageObject::CreateImage2D(UGPUContextObject* contextObject,
									int32 width,
									int32 height,
									EGpuPixelFormat format,
									bool readOnly,
									bool writeOnly)
{
	if (!contextObject || !contextObject->GetContext() || width <= 0 || height <= 0)
	{
		return false;
	}

	Gpu::ImageDescription Desc;
	Desc.Type = Gpu::ImageType::Tex2D;
	Desc.Format = ToNativePixelFormat(format);
	Desc.Width = static_cast<uint32>(width);
	Desc.Height = static_cast<uint32>(height);
	Desc.DepthOrLayers = 1;

	if (readOnly)
	{
		Desc.AccessMode = Gpu::Access::ReadOnly;
	}
	else if (writeOnly)
	{
		Desc.AccessMode = Gpu::Access::WriteOnly;
	}
	else
	{
		Desc.AccessMode = Gpu::Access::ReadWrite;
	}

	mpImage = contextObject->GetContext()->CreateImage(Desc);
	return mpImage != nullptr;
}

bool UGPUImageObject::CreateImage2DArray(UGPUContextObject* contextObject,
										 int32 width,
										 int32 height,
										 int32 layers,
										 EGpuPixelFormat format,
										 bool readOnly,
										 bool writeOnly)
{
	if (!contextObject || !contextObject->GetContext() || width <= 0 || height <= 0 || layers <= 0)
	{
		return false;
	}

	Gpu::ImageDescription Desc;
	Desc.Type = Gpu::ImageType::Tex2DArray;
	Desc.Format = ToNativePixelFormat(format);
	Desc.Width = static_cast<uint32>(width);
	Desc.Height = static_cast<uint32>(height);
	Desc.DepthOrLayers = static_cast<uint32>(layers);

	if (readOnly)
	{
		Desc.AccessMode = Gpu::Access::ReadOnly;
	}
	else if (writeOnly)
	{
		Desc.AccessMode = Gpu::Access::WriteOnly;
	}
	else
	{
		Desc.AccessMode = Gpu::Access::ReadWrite;
	}

	mpImage = contextObject->GetContext()->CreateImage(Desc);
	return mpImage != nullptr;
}

bool UGPUImageObject::CreateImage3D(UGPUContextObject* contextObject,
									int32 width,
									int32 height,
									int32 depth,
									EGpuPixelFormat format,
									bool readOnly,
									bool writeOnly)
{
	if (!contextObject || !contextObject->GetContext() || width <= 0 || height <= 0 || depth <= 0)
	{
		return false;
	}

	Gpu::ImageDescription Desc;
	Desc.Type = Gpu::ImageType::Tex3D;
	Desc.Format = ToNativePixelFormat(format);
	Desc.Width = static_cast<uint32>(width);
	Desc.Height = static_cast<uint32>(height);
	Desc.DepthOrLayers = static_cast<uint32>(depth);

	if (readOnly)
	{
		Desc.AccessMode = Gpu::Access::ReadOnly;
	}
	else if (writeOnly)
	{
		Desc.AccessMode = Gpu::Access::WriteOnly;
	}
	else
	{
		Desc.AccessMode = Gpu::Access::ReadWrite;
	}

	mpImage = contextObject->GetContext()->CreateImage(Desc);
	return mpImage != nullptr;
}

bool UGPUImageObject::IsValidImage() const
{
	return mpImage != nullptr;
}

int32 UGPUImageObject::GetWidth() const
{
	return mpImage ? static_cast<int32>(mpImage->GetWidth()) : 0;
}

int32 UGPUImageObject::GetHeight() const
{
	return mpImage ? static_cast<int32>(mpImage->GetHeight()) : 0;
}

int32 UGPUImageObject::GetDepthOrLayers() const
{
	return mpImage ? static_cast<int32>(mpImage->GetDepthOrLayers()) : 0;
}

EGpuImageType UGPUImageObject::GetImageType() const
{
	return mpImage ? FromNativeImageType(mpImage->GetType()) : EGpuImageType::Tex2D;
}

EGpuPixelFormat UGPUImageObject::GetPixelFormat() const
{
	return mpImage ? FromNativePixelFormat(mpImage->GetFormat()) : EGpuPixelFormat::Unknown;
}

bool UGPUImageObject::UploadBytes(UGPUContextObject* ContextObject,
								  const TArray<uint8>& Bytes)
{
	if (!ValidateContextAndQueue(ContextObject))
    {
        return false;
    }

    const size_t Bpp = GetBytesPerPixel(mpImage->GetFormat());
    if (Bpp == 0)
    {
        return false;
    }

	Gpu::ImageRegion Region;
    switch (mpImage->GetType())
    {
		case Gpu::ImageType::Tex2D:
		{
			Region = {0, 0, 0, mpImage->GetWidth(), mpImage->GetHeight(), 1};
			break;
		}
		case Gpu::ImageType::Tex2DArray:
		case Gpu::ImageType::Tex3D:
		{
			Region = {0, 0, 0, mpImage->GetWidth(), mpImage->GetHeight(), mpImage->GetDepthOrLayers()};
			break;
		}
		default:
			return false;
    }

    return mpImage->Upload(*ContextObject->GetDefaultQueue(),
						 Bytes.GetData(),
						 static_cast<size_t>(Bytes.Num()),
						 Region);
}

bool UGPUImageObject::DownloadBytes(UGPUContextObject* contextObject,
									TArray<uint8>& bytes)
{
	TArray64<uint8> temp;
	if (!DownloadToCpuBytes(contextObject, temp))
	{
		return false;
	}

	bytes.SetNumUninitialized(static_cast<int32>(temp.Num()));
	FMemory::Memcpy(bytes.GetData(), temp.GetData(), temp.Num());
	return true;
}

bool UGPUImageObject::FillColor(UGPUContextObject* contextObject,
								const FColor& color)
{
	if (!ValidateContextAndQueue(contextObject))
	{
		return false;
	}
	const size_t Bpp = GetBytesPerPixel(mpImage->GetFormat());
	if (Bpp == 0)
	{
		return false;
	}

	Gpu::ImageRegion region;
	switch (mpImage->GetType())
	{
		case Gpu::ImageType::Tex2D:
		{
			region = { 0, 0, 0, mpImage->GetWidth(), mpImage->GetHeight(), 1 };
			break;
		}
		case Gpu::ImageType::Tex2DArray:
		case Gpu::ImageType::Tex3D:
		{
			region = { 0, 0, 0, mpImage->GetWidth(), mpImage->GetHeight(), mpImage->GetDepthOrLayers() };
			break;
		}
		default:
			return false;
	}

	const uint32_t channelCount = GetChannelCount(mpImage->GetFormat());
	std::vector<uint8_t> colorData;
	for (uint8_t i = 0; i < GetChannelCount(mpImage->GetFormat()); ++i)
	{
		uint8_t channelValue = 0;
		switch (i)
		{
			case 0:
				channelValue = color.R;
				break;
			case 1:
				channelValue = color.G;
				break;
			case 2:
				channelValue = color.B;
				break;
			case 3:
				channelValue = color.A;
				break;
			default:
				break;
		}
		colorData.push_back(channelValue);
	}

	return mpImage->Fill(*contextObject->GetDefaultQueue(),
						 colorData.data(),
						 channelCount * sizeof(uint8_t),
						 region);
}

UTexture2D* UGPUImageObject::CreateTexture2D(UGPUContextObject* contextObject,
											 bool isSRGB,
											 bool generateMips)
{
	check(IsInGameThread());

	if (!ValidateContextAndQueue(contextObject) || mpImage->GetType() != Gpu::ImageType::Tex2D)
	{
		return nullptr;
	}

	const EPixelFormat format = ToUEPixelFormat(mpImage->GetFormat());
	const size_t bpp = GetBytesPerPixel(mpImage->GetFormat());
	if (format == PF_Unknown)
	{
		return nullptr;
	}

	TArray64<uint8> bytes;
	std::vector<RenderUtils::Mip> mips;
	if (!DownloadAndBuildMips(contextObject, generateMips, bytes, mips))
	{
		return nullptr;
	}

	UTexture2D* texture = UTexture2D::CreateTransient(static_cast<int32>(mpImage->GetWidth()),
													  static_cast<int32>(mpImage->GetHeight()),
													  format);

	if (!texture)
	{
		FreeGeneratedMipChain(mips);
		return nullptr;
	}

	texture->NeverStream = true;
	texture->SRGB = isSRGB;
	texture->MipGenSettings = generateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;

	texture->SetPlatformData(new FTexturePlatformData());

	const bool filled = FillPlatformDataFromMips(texture->GetPlatformData(),
												 format,
												 mips,
												 bpp,
												 mpImage->GetWidth(),
												 mpImage->GetHeight(),
												 1);

	FreeGeneratedMipChain(mips);

	if (!filled)
	{
		return nullptr;
	}

	texture->UpdateResource();
	return texture;
}

UTextureRenderTarget2D* UGPUImageObject::CreateAndWriteRenderTarget2D(UGPUContextObject* contextObject,
																	  FLinearColor clearColor,
																	  bool isSRGB,
																	  bool generateMips)
{
	if (!ValidateContextAndQueue(contextObject) || mpImage->GetType() != Gpu::ImageType::Tex2D)
	{
		return nullptr;
	}

	const ETextureRenderTargetFormat format = ToUERenderTargetPixelFormat(mpImage->GetFormat());
	if (format == -1)
	{
		return nullptr;
	}

	TObjectPtr<UTextureRenderTarget2D> texture = NewObject<UTextureRenderTarget2D>(GetTransientPackage(),
																				   NAME_None,
																				   RF_Transient);

	if (!texture)
	{
		return nullptr;
	}

	texture->RenderTargetFormat = format;
	texture->ClearColor = clearColor;
	texture->bAutoGenerateMips = false;
	texture->bCanCreateUAV = true;
	texture->InitAutoFormat(static_cast<int32>(mpImage->GetWidth()), static_cast<int32>(mpImage->GetHeight()));
	texture->UpdateResourceImmediate(true);

	UTexture2D* temp = CreateTexture2D(contextObject, isSRGB, generateMips);
	if (!RenderUtils::BlitTextureToRenderTarget_Immediate(temp, texture))
	{
		return nullptr;
	}
	return texture;
}

UTexture2DArray* UGPUImageObject::CreateTexture2DArray(UGPUContextObject* contextObject,
													   bool isSRGB,
													   bool generateMips)
{
	if (!ValidateContextAndQueue(contextObject) || mpImage->GetType() != Gpu::ImageType::Tex2DArray)
    {
        return nullptr;
    }

	const EPixelFormat format = ToUEPixelFormat(mpImage->GetFormat());
	const size_t bpp = GetBytesPerPixel(mpImage->GetFormat());
    if (format == PF_Unknown)
    {
        return nullptr;
    }

	TArray64<uint8> bytes;
	std::vector<RenderUtils::Mip> mips;
	if (!DownloadAndBuildMips(contextObject, generateMips, bytes, mips))
	{
		return nullptr;
	}

    UTexture2DArray* texture = NewObject<UTexture2DArray>(GetTransientPackage(), NAME_None, RF_Transient);
    if (!texture)
    {
		FreeGeneratedMipChain(mips);
        return nullptr;
    }

    texture->NeverStream = true;
    texture->SRGB = isSRGB;
	texture->MipGenSettings = generateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;

    texture->SetPlatformData(new FTexturePlatformData());

	const bool filled = FillPlatformDataFromMips(texture->GetPlatformData(),
												 format,
												 mips,
												 bpp,
												 mpImage->GetWidth(),
												 mpImage->GetHeight(),
												 mpImage->GetDepthOrLayers());

	FreeGeneratedMipChain(mips);

	if (!filled)
	{
		return nullptr;
	}

	texture->UpdateResource();
	return texture;
}

UVolumeTexture* UGPUImageObject::CreateVolumeTexture(UGPUContextObject* contextObject,
													 bool isSRGB,
													 bool generateMips)
{
	if (!ValidateContextAndQueue(contextObject) || mpImage->GetType() != Gpu::ImageType::Tex3D)
	{
		return nullptr;
	}

	const EPixelFormat format = ToUEPixelFormat(mpImage->GetFormat());
	const size_t bpp = GetBytesPerPixel(mpImage->GetFormat());
	if (format == PF_Unknown)
	{
		return nullptr;
	}

	TArray64<uint8> bytes;
	std::vector<RenderUtils::Mip> mips;
	if (!DownloadAndBuildMips(contextObject, generateMips, bytes, mips))
	{
		return nullptr;
	}

	UVolumeTexture* texture = NewObject<UVolumeTexture>(GetTransientPackage(),
														NAME_None,
														RF_Transient);
	if (!texture)
	{
		FreeGeneratedMipChain(mips);
		return nullptr;
	}

	texture->NeverStream = true;
	texture->SRGB = isSRGB;
	texture->MipGenSettings = generateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;
	texture->SetPlatformData(new FTexturePlatformData());

	const bool filled = FillPlatformDataFromMips(texture->GetPlatformData(),
												 format,
												 mips,
												 bpp,
												 mpImage->GetWidth(),
												 mpImage->GetHeight(),
												 mpImage->GetDepthOrLayers());

	FreeGeneratedMipChain(mips);

	if (!filled)
	{
		return nullptr;
	}

	texture->UpdateResource();
	return texture;
}

bool UGPUImageObject::WriteToRenderTarget2D(UGPUContextObject* contextObject,
											UTextureRenderTarget2D* output)
{
	bool isSRGB = output->IsSRGB();
	UTexture2D* temp = CreateTexture2D(contextObject, isSRGB, false);

	TObjectPtr<UTextureRenderTarget2D> texture(output);
	if (!RenderUtils::BlitTextureToRenderTarget_Immediate(temp, texture))
	{
		return false;
	}
	return true;
}

bool UGPUImageObject::UpdateTexture2D(UGPUContextObject* contextObject,
									  UTexture2D* texture)
{
	if (!ValidateContextAndQueue(contextObject) ||
		!texture ||
		!texture->GetPlatformData() ||
		mpImage->GetType() != Gpu::ImageType::Tex2D)
	{
		return false;
	}

	const bool generateMips = texture->MipGenSettings != TMGS_NoMipmaps;
	if (UpdateTexture_Internal(texture->GetPlatformData(), contextObject, generateMips))
	{
		texture->UpdateResource();
		return true;
	}
	return false;
}

bool UGPUImageObject::UpdateTexture2DAsync(UGPUContextObject* contextObject,
										   UTexture2D* texture,
										   const std::function<void(bool)>& completionCallback,
										   ENamedThreads::Type callbackThread)
{
	if (!ValidateContextAndQueue(contextObject) ||
		!texture ||
		mpImage->GetType() != Gpu::ImageType::Tex2D)
	{
		if (completionCallback)
		{
			completionCallback(false);
		}
		return false;
	}

	const EPixelFormat format = ToUEPixelFormat(mpImage->GetFormat());
	const size_t bpp = GetBytesPerPixel(mpImage->GetFormat());

	if (format == PF_Unknown || bpp == 0)
	{
		AsyncTask(callbackThread, [completionCallback]()
		{
			if (completionCallback)
			{
				completionCallback(false);
			}
		});
		return false;
	}

	FTexturePlatformData* platformData = texture->GetPlatformData();
	if (platformData->SizeX != static_cast<int32>(mpImage->GetWidth()) ||
		platformData->SizeY != static_cast<int32>(mpImage->GetHeight()) ||
		platformData->PixelFormat != format)
	{
		AsyncTask(callbackThread, [completionCallback]()
		{
			if (completionCallback)
			{
				completionCallback(false);
			}
		});
		return false;
	}

	const uint32 width = mpImage->GetWidth();
	const uint32 height = mpImage->GetHeight();

	const size_t totalBytes = static_cast<size_t>(width) *
							  static_cast<size_t>(height) *
							  bpp;

	TSharedPtr<FGPUTexture2DAsyncUpdate> state = MakeShared<FGPUTexture2DAsyncUpdate>();
	state->mpTexture = texture;
	state->mBaseBytes.resize(totalBytes, 0);
	state->mCompletionCallback = std::move(completionCallback);

	Gpu::ImageRegion region;
	region.X = 0;
	region.Y = 0;
	region.Z = 0;
	region.Width = width;
	region.Height = height;
	region.Depth = 1;

	std::shared_ptr<Gpu::IEvent> downloadEvent = mpImage->DownloadAsync(*contextObject->GetDefaultQueue(),
																		state->mBaseBytes.data(),
																		state->mBaseBytes.size(),
																		region);

	if (!downloadEvent)
	{
		AsyncTask(callbackThread, [state]()
		{
			if (state->mCompletionCallback)
			{
				state->mCompletionCallback(false);
			}
		});
		return false;
	}

	TWeakObjectPtr<UGPUImageObject> weakPtr(this);
	TWeakObjectPtr<UGPUContextObject> weakContextPtr(contextObject);

	downloadEvent->SetCompletionCallback([weakPtr, weakContextPtr, state, bpp, callbackThread]()
	{
		AsyncTask(callbackThread, [weakPtr, weakContextPtr, state, bpp, callbackThread]()
		{
			UGPUImageObject* selfPtr = weakPtr.Get();
			if (!selfPtr || !selfPtr->mpImage || !state->mpTexture.IsValid())
			{
				AsyncTask(callbackThread, [state]()
				{
					if (state->mCompletionCallback)
					{
						state->mCompletionCallback(false);
					}
				});
				return;
			}

			UTexture2D* texture = state->mpTexture.Get();
			FTexturePlatformData* platformData = texture->GetPlatformData();
			if (!platformData)
			{
				AsyncTask(ENamedThreads::GameThread, [state]()
				{
					if (state->mCompletionCallback)
					{
						state->mCompletionCallback(false);
					}
				});
				return;
			}

			const bool generateMips = texture->MipGenSettings != TMGS_NoMipmaps;
			if (generateMips)
			{
				if (!selfPtr->GenerateMipChain(selfPtr->mpImage->GetFormat(),
											   state->mBaseBytes.data(),
											   selfPtr->mpImage->GetWidth(),
											   selfPtr->mpImage->GetHeight(),
											   1,
											   state->mMips))
				{
					AsyncTask(callbackThread, [state]()
					{
						if (state->mCompletionCallback)
						{
							state->mCompletionCallback(false);
						}
					});
					return;
				}
			}
			else
			{
				RenderUtils::Mip baseMip;
				baseMip.mWidth = selfPtr->mpImage->GetWidth();
				baseMip.mHeight = selfPtr->mpImage->GetHeight();
				baseMip.mSlices = 1;
				baseMip.mChannels = selfPtr->GetChannelCount(selfPtr->mpImage->GetFormat());
				baseMip.mPixels = state->mBaseBytes.data();
				state->mMips.push_back(baseMip);
			}

			if (platformData->Mips.Num() != static_cast<int32>(state->mMips.size()))
			{
				selfPtr->FreeGeneratedMipChain(state->mMips);

				AsyncTask(ENamedThreads::GameThread, [state]()
				{
					if (state->mCompletionCallback)
					{
						state->mCompletionCallback(false);
					}
				});
				return;
			}

			state->mRegions.resize(static_cast<int32>(state->mMips.size()));
			state->mSrcPitches.resize(static_cast<int32>(state->mMips.size()));
			state->mSrcBpps.resize(static_cast<int32>(state->mMips.size()));

			for (int32 mipIndex = 0; mipIndex < static_cast<int32>(state->mMips.size()); ++mipIndex)
			{
				const RenderUtils::Mip& Mip = state->mMips[mipIndex];

				state->mRegions[mipIndex] = FUpdateTextureRegion2D(0,
																   0,
																   0,
																   0,
																   static_cast<uint32>(Mip.mWidth),
																   static_cast<uint32>(Mip.mHeight));

				state->mSrcPitches[mipIndex] = static_cast<uint32>(Mip.mWidth * bpp);
				state->mSrcBpps[mipIndex] = static_cast<uint32>(bpp);
			}

			for (int32 mipIndex = 0; mipIndex < static_cast<int32>(state->mMips.size()); ++mipIndex)
			{
				const RenderUtils::Mip& Mip = state->mMips[mipIndex];

				texture->UpdateTextureRegions(mipIndex,
											  1,
											  &state->mRegions[mipIndex],
											  state->mSrcPitches[mipIndex],
											  state->mSrcBpps[mipIndex],
											  reinterpret_cast<uint8*>(Mip.mPixels),
				[weakPtr, state, mipIndex, callbackThread](uint8* SrcData, const FUpdateTextureRegion2D* Regions)
				{
					if (mipIndex == static_cast<int32>(state->mMips.size()) - 1)
					{
						if (UGPUImageObject* Self = weakPtr.Get())
						{
							Self->FreeGeneratedMipChain(state->mMips);
						}

						AsyncTask(callbackThread, [state]()
						{
							if (state->mCompletionCallback)
							{
								state->mCompletionCallback(true);
							}
						});
					}
				});
			}
		});
	});

	return true;
}

bool UGPUImageObject::UpdateTexture2DArray(UGPUContextObject* contextObject,
										   UTexture2DArray* texture)
{
	if (!ValidateContextAndQueue(contextObject) ||
		!texture ||
		!texture->GetPlatformData() ||
		mpImage->GetType() != Gpu::ImageType::Tex2DArray)
	{
		return false;
	}

	const bool generateMips = texture->MipGenSettings != TMGS_NoMipmaps;
	if (UpdateTexture_Internal(texture->GetPlatformData(), contextObject, generateMips))
	{
		texture->UpdateResource();
		return true;
	}
	return false;
}

bool UGPUImageObject::UpdateVolumeTexture(UGPUContextObject* contextObject,
										  UVolumeTexture* texture)
{
	if (!ValidateContextAndQueue(contextObject) ||
		!texture ||
		!texture->GetPlatformData() ||
		mpImage->GetType() != Gpu::ImageType::Tex3D)
	{
		return false;
	}

	const bool generateMips = texture->MipGenSettings != TMGS_NoMipmaps;
	if (UpdateTexture_Internal(texture->GetPlatformData(), contextObject, generateMips))
	{
		texture->UpdateResource();
		return true;
	}
	return false;
}

Gpu::PixelFormat UGPUImageObject::ToNativePixelFormat(EGpuPixelFormat format) const
{
	switch (format)
	{
		case EGpuPixelFormat::R8:
			return Gpu::PixelFormat::R8;
		case EGpuPixelFormat::RG8:
			return Gpu::PixelFormat::RG8;
		case EGpuPixelFormat::RGBA8:
			return Gpu::PixelFormat::RGBA8;
		case EGpuPixelFormat::R16F:
			return Gpu::PixelFormat::R16F;
		case EGpuPixelFormat::RG16F:
			return Gpu::PixelFormat::RG16F;
		case EGpuPixelFormat::RGBA16F:
			return Gpu::PixelFormat::RGBA16F;
		case EGpuPixelFormat::R32F:
			return Gpu::PixelFormat::R32F;
		case EGpuPixelFormat::RG32F:
			return Gpu::PixelFormat::RG32F;
		case EGpuPixelFormat::RGBA32F:
			return Gpu::PixelFormat::RGBA32F;
		case EGpuPixelFormat::R32U:
			return Gpu::PixelFormat::R32U;
		case EGpuPixelFormat::RG32U:
			return Gpu::PixelFormat::RG32U;
		case EGpuPixelFormat::RGBA32U:
			return Gpu::PixelFormat::RGBA32U;
		case EGpuPixelFormat::R32S:
			return Gpu::PixelFormat::R32S;
		default:
			return Gpu::PixelFormat::Unknown;
	}
}

EGpuPixelFormat UGPUImageObject::FromNativePixelFormat(Gpu::PixelFormat format) const
{
	switch (format)
	{
		case Gpu::PixelFormat::R8:
			return EGpuPixelFormat::R8;
		case Gpu::PixelFormat::RG8:
			return EGpuPixelFormat::RG8;
		case Gpu::PixelFormat::RGBA8:
			return EGpuPixelFormat::RGBA8;
		case Gpu::PixelFormat::R16F:
			return EGpuPixelFormat::R16F;
		case Gpu::PixelFormat::RG16F:
			return EGpuPixelFormat::RG16F;
		case Gpu::PixelFormat::RGBA16F:
			return EGpuPixelFormat::RGBA16F;
		case Gpu::PixelFormat::R32F:
			return EGpuPixelFormat::R32F;
		case Gpu::PixelFormat::RG32F:
			return EGpuPixelFormat::RG32F;
		case Gpu::PixelFormat::RGBA32F:
			return EGpuPixelFormat::RGBA32F;
		case Gpu::PixelFormat::R32U:
			return EGpuPixelFormat::R32U;
		case Gpu::PixelFormat::RG32U:
			return EGpuPixelFormat::RG32U;
		case Gpu::PixelFormat::RGBA32U:
			return EGpuPixelFormat::RGBA32U;
		case Gpu::PixelFormat::R32S:
			return EGpuPixelFormat::R32S;
		default:
			return EGpuPixelFormat::Unknown;
	}
}

EGpuImageType UGPUImageObject::FromNativeImageType(Gpu::ImageType type) const
{
	switch (type)
	{
		case Gpu::ImageType::Tex2D:
			return EGpuImageType::Tex2D;
		case Gpu::ImageType::Tex2DArray:
			return EGpuImageType::Tex2DArray;
		case Gpu::ImageType::Tex3D:
			return EGpuImageType::Tex3D;
		default:
			return EGpuImageType::Tex2D;
	}
}

EPixelFormat UGPUImageObject::ToUEPixelFormat(Gpu::PixelFormat format) const
{
	switch (format)
	{
		case Gpu::PixelFormat::R8:
			return PF_G8;
		case Gpu::PixelFormat::RG8:
			return PF_R8G8;
		case Gpu::PixelFormat::RGBA8:
			return PF_R8G8B8A8;
		case Gpu::PixelFormat::R16F:
			return PF_R16F;
		case Gpu::PixelFormat::RG16F:
			return PF_G16R16F;
		case Gpu::PixelFormat::RGBA16F:
			return PF_FloatRGBA;
		case Gpu::PixelFormat::R32F:
			return PF_R32_FLOAT;
		case Gpu::PixelFormat::RG32F:
			return PF_G32R32F;
		case Gpu::PixelFormat::RGBA32F:
			return PF_A32B32G32R32F;
		case Gpu::PixelFormat::R32U:
			return PF_R32_UINT;
		case Gpu::PixelFormat::RG32U:
			return PF_R32G32_UINT;
		case Gpu::PixelFormat::RGBA32U:
			return PF_R32G32B32A32_UINT;
		case Gpu::PixelFormat::R32S:
			return PF_R32_SINT;
		default:
			return PF_Unknown;
	}
}

ETextureRenderTargetFormat UGPUImageObject::ToUERenderTargetPixelFormat(Gpu::PixelFormat format) const
{
	switch (format)
	{
		case Gpu::PixelFormat::R8:
			return RTF_R8;
		case Gpu::PixelFormat::RG8:
			return RTF_RG8;
		case Gpu::PixelFormat::RGBA8:
			return RTF_RGBA8;
		case Gpu::PixelFormat::R16F:
			return RTF_R16f;
		case Gpu::PixelFormat::RG16F:
			return RTF_RG16f;
		case Gpu::PixelFormat::RGBA16F:
			return RTF_RGBA16f;
		case Gpu::PixelFormat::R32F:
			return RTF_R32f;
		case Gpu::PixelFormat::RG32F:
			return RTF_RG32f;
		case Gpu::PixelFormat::RGBA32F:
			return RTF_RGBA32f;
		default:
			return ETextureRenderTargetFormat(-1);
	}
}

size_t UGPUImageObject::GetBytesPerPixel(Gpu::PixelFormat format) const
{
	switch (format)
	{
		case Gpu::PixelFormat::R8:
			return 1;
		case Gpu::PixelFormat::RG8:
			return 2;
		case Gpu::PixelFormat::RGBA8:
			return 4;
		case Gpu::PixelFormat::R16F:
			return 2;
		case Gpu::PixelFormat::RG16F:
			return 4;
		case Gpu::PixelFormat::RGBA16F:
			return 8;
		case Gpu::PixelFormat::R32F:
			return 4;
		case Gpu::PixelFormat::RG32F:
			return 8;
		case Gpu::PixelFormat::RGBA32F:
			return 16;
		case Gpu::PixelFormat::R32U:
			return 4;
		case Gpu::PixelFormat::RG32U:
			return 8;
		case Gpu::PixelFormat::RGBA32U:
			return 16;
		case Gpu::PixelFormat::R32S:
			return 4;
		default:
			return 0;
	}
}

bool UGPUImageObject::ValidateContextAndQueue(UGPUContextObject* contextObject) const
{
	return contextObject &&
           contextObject->GetContext() &&
           contextObject->GetDefaultQueue() &&
           mpImage != nullptr;
}

uint32 UGPUImageObject::GetChannelCount(Gpu::PixelFormat format) const
{
	switch (format)
	{
		case Gpu::PixelFormat::R8:
		case Gpu::PixelFormat::R16F:
		case Gpu::PixelFormat::R32F:
		case Gpu::PixelFormat::R32U:
		case Gpu::PixelFormat::R32S:
		{
			return 1;
		}
		case Gpu::PixelFormat::RG8:
		case Gpu::PixelFormat::RG16F:
		case Gpu::PixelFormat::RG32F:
		case Gpu::PixelFormat::RG32U:
		{
			return 2;
		}
		case Gpu::PixelFormat::RGBA8:
		case Gpu::PixelFormat::RGBA16F:
		case Gpu::PixelFormat::RGBA32F:
		case Gpu::PixelFormat::RGBA32U:
		{
			return 4;
		}
		default:
			return 0;
	}
}

bool UGPUImageObject::GenerateMipChain(Gpu::PixelFormat format,
									   uint8_t* srcBytes,
									   uint32_t width,
									   uint32_t height,
									   uint32_t layers,
									   std::vector<RenderUtils::Mip>& output)
{
	const uint32 channels = GetChannelCount(format);
	if (!srcBytes || channels == 0)
	{
		return false;
	}

	switch (format)
	{
		case Gpu::PixelFormat::R8:
		case Gpu::PixelFormat::RG8:
		case Gpu::PixelFormat::RGBA8:
		{
			RenderUtils::GenerateMipsUInt8(output,
										   reinterpret_cast<uint8_t*>(srcBytes),
										   width,
										   height,
										   layers,
										   channels);
			return true;
		}
		case Gpu::PixelFormat::R16F:
		case Gpu::PixelFormat::RG16F:
		case Gpu::PixelFormat::RGBA16F:
		{
			RenderUtils::GenerateMipsFloat16(output,
											 reinterpret_cast<FFloat16*>(srcBytes),
											 width,
											 height,
											 layers,
											 channels);
			return true;
		}
		case Gpu::PixelFormat::R32F:
		case Gpu::PixelFormat::RG32F:
		case Gpu::PixelFormat::RGBA32F:
		{
			RenderUtils::GenerateMipsFloat(output,
										   reinterpret_cast<float*>(srcBytes),
										   width,
										   height,
										   layers,
										   channels);
			return true;
		}
		case Gpu::PixelFormat::R32U:
		case Gpu::PixelFormat::RG32U:
		case Gpu::PixelFormat::RGBA32U:
		{
			RenderUtils::GenerateMipsUInt32(output,
											reinterpret_cast<uint32_t*>(srcBytes),
											width,
											height,
											layers,
											channels);
			return true;
		}
		case Gpu::PixelFormat::R32S:
		{
			RenderUtils::GenerateMipsInt32(output,
										   reinterpret_cast<int32_t*>(srcBytes),
										   width,
										   height,
										   layers,
										   channels);
			return true;
		}
		default:
		{
			return false;
		}
	}
}

void UGPUImageObject::FreeGeneratedMipChain(std::vector<RenderUtils::Mip>& mips)
{
	// Mip 0 points into the downloaded CPU byte array.
	// Mips 1+ are allocated with new[] inside GenerateMip.
	for (size_t i = 1; i < mips.size(); ++i)
	{
		delete[] mips[i].mPixels;
		mips[i].mPixels = nullptr;
	}

	mips.clear();
}

bool UGPUImageObject::FillPlatformDataFromMips(FTexturePlatformData* platformData,
											   EPixelFormat format,
											   const std::vector<RenderUtils::Mip>& mips,
											   size_t bytesPerPixel,
											   uint32_t baseWidth,
											   uint32_t baseHeight,
											   uint32_t layers)
{
	if (!platformData || mips.empty() || bytesPerPixel == 0)
	{
		return false;
	}

	platformData->SizeX = baseWidth;
	platformData->SizeY = baseHeight;
	platformData->SetNumSlices(layers);
	platformData->PixelFormat = format;
	platformData->Mips.Empty();

	for (const RenderUtils::Mip& srcMip : mips)
	{
		FTexture2DMipMap* dstMip = new FTexture2DMipMap();
		platformData->Mips.Add(dstMip);

		dstMip->SizeX = static_cast<int32>(srcMip.mWidth);
		dstMip->SizeY = static_cast<int32>(srcMip.mHeight);
		dstMip->SizeZ = static_cast<int32>(srcMip.mSlices);

		const int64 MipBytes = static_cast<int64>(srcMip.mWidth) *
							   static_cast<int64>(srcMip.mHeight) *
							   static_cast<int64>(srcMip.mSlices) *
							   static_cast<int64>(bytesPerPixel);

		dstMip->BulkData.Lock(LOCK_READ_WRITE);
		void* dst = dstMip->BulkData.Realloc(MipBytes);
		FMemory::Memcpy(dst, srcMip.mPixels, MipBytes);
		dstMip->BulkData.Unlock();
	}

	return true;
}

bool UGPUImageObject::DownloadAndBuildMips(UGPUContextObject* contextObject,
										   bool benerateMips,
										   TArray64<uint8_t>& outputBytes,
										   std::vector<RenderUtils::Mip>& outputMips)
{
	if (!DownloadToCpuBytes(contextObject, outputBytes))
	{
		return false;
	}

	const uint32 width = mpImage->GetWidth();
	const uint32 height = mpImage->GetHeight();
	const uint32 layers = mpImage->GetType() == Gpu::ImageType::Tex2D ? 1 : mpImage->GetDepthOrLayers();

	if (benerateMips)
	{
		return GenerateMipChain(mpImage->GetFormat(),
								outputBytes.GetData(),
								width,
								height,
								layers,
								outputMips);
	}

	const uint32 channels = GetChannelCount(mpImage->GetFormat());

	RenderUtils::Mip baseMip;
	baseMip.mWidth = width;
	baseMip.mHeight = height;
	baseMip.mSlices = layers;
	baseMip.mChannels = channels;
	baseMip.mPixels = outputBytes.GetData();

	outputMips.clear();
	outputMips.push_back(baseMip);
	return true;
}

bool UGPUImageObject::DownloadToCpuBytes(UGPUContextObject* contextObject,
										 TArray64<uint8>& output) const
{
	if (!ValidateContextAndQueue(contextObject))
    {
        return false;
    }

    const size_t bpp = GetBytesPerPixel(mpImage->GetFormat());
    if (bpp == 0)
    {
        return false;
    }

	Gpu::ImageRegion region;
    size_t totalBytes = 0;

    switch (mpImage->GetType())
    {
		case Gpu::ImageType::Tex2D:
		{
		    region = {0, 0, 0, mpImage->GetWidth(), mpImage->GetHeight(), 1};
		    totalBytes = static_cast<size_t>(mpImage->GetWidth()) * mpImage->GetHeight() * bpp;
		    break;
		}
		case Gpu::ImageType::Tex2DArray:
		case Gpu::ImageType::Tex3D:
		{
		    region = {0, 0, 0, mpImage->GetWidth(), mpImage->GetHeight(), mpImage->GetDepthOrLayers()};
		    totalBytes = static_cast<size_t>(mpImage->GetWidth()) * mpImage->GetHeight() * mpImage->GetDepthOrLayers() * bpp;
		    break;
		}
		default:
			return false;
    }

    output.SetNumUninitialized(totalBytes);

    return mpImage->Download(*contextObject->GetDefaultQueue(),
						   output.GetData(),
						   totalBytes,
						   region);
}

bool UGPUImageObject::UpdateTexture_Internal(FTexturePlatformData* platformData,
											 UGPUContextObject* contextObject,
											 bool generateMips)
{
	TArray64<uint8> bytes;
	std::vector<RenderUtils::Mip> mips;
	if (!DownloadAndBuildMips(contextObject, generateMips, bytes, mips))
	{
		return false;
	}

	if (platformData->Mips.Num() != static_cast<int32>(mips.size()))
	{
		FreeGeneratedMipChain(mips);
		return false;
	}

	const size_t bpp = GetBytesPerPixel(mpImage->GetFormat());
	for (int32 mipIndex = 0; mipIndex < platformData->Mips.Num(); ++mipIndex)
	{
		const RenderUtils::Mip& src = mips[mipIndex];
		FTexture2DMipMap& dst = platformData->Mips[mipIndex];

		const int64 MipBytes = static_cast<int64>(src.mWidth) *
							   static_cast<int64>(src.mHeight) *
							   static_cast<int64>(src.mSlices) *
							   static_cast<int64>(bpp);

		void* dstData = dst.BulkData.Lock(LOCK_READ_WRITE);
		if (!dstData)
		{
			FreeGeneratedMipChain(mips);
			return false;
		}

		if (dst.BulkData.GetBulkDataSize() != MipBytes)
		{
			dst.BulkData.Unlock();
			FreeGeneratedMipChain(mips);
			return false;
		}

		FMemory::Memcpy(dstData, src.mPixels, MipBytes);
		dst.BulkData.Unlock();
	}

	FreeGeneratedMipChain(mips);
	return true;
}