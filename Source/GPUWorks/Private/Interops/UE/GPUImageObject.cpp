#include "Interops/UE/GPUImageObject.h"
#include "Interops/UE/GPUContextObject.h"

#include "Engine/Texture.h"

bool UGPUImageObject::CreateImage2D(UGPUContextObject* ContextObject,
									int32 Width,
									int32 Height,
									EGpuPixelFormat Format,
									bool bReadOnly,
									bool bWriteOnly)
{
	if (!ContextObject || !ContextObject->GetContext() || Width <= 0 || Height <= 0)
	{
		return false;
	}

	Gpu::ImageDescription Desc;
	Desc.Type = Gpu::ImageType::Tex2D;
	Desc.Format = ToNativePixelFormat(Format);
	Desc.Width = static_cast<uint32>(Width);
	Desc.Height = static_cast<uint32>(Height);
	Desc.DepthOrLayers = 1;

	if (bReadOnly)
	{
		Desc.AccessMode = Gpu::Access::ReadOnly;
	}
	else if (bWriteOnly)
	{
		Desc.AccessMode = Gpu::Access::WriteOnly;
	}
	else
	{
		Desc.AccessMode = Gpu::Access::ReadWrite;
	}

	Image = ContextObject->GetContext()->CreateImage(Desc);
	return Image != nullptr;
}

bool UGPUImageObject::CreateImage2DArray(UGPUContextObject* ContextObject,
										 int32 Width,
										 int32 Height,
										 int32 Layers,
										 EGpuPixelFormat Format,
										 bool bReadOnly,
										 bool bWriteOnly)
{
	if (!ContextObject || !ContextObject->GetContext() || Width <= 0 || Height <= 0 || Layers <= 0)
	{
		return false;
	}

	Gpu::ImageDescription Desc;
	Desc.Type = Gpu::ImageType::Tex2DArray;
	Desc.Format = ToNativePixelFormat(Format);
	Desc.Width = static_cast<uint32>(Width);
	Desc.Height = static_cast<uint32>(Height);
	Desc.DepthOrLayers = static_cast<uint32>(Layers);

	if (bReadOnly)
	{
		Desc.AccessMode = Gpu::Access::ReadOnly;
	}
	else if (bWriteOnly)
	{
		Desc.AccessMode = Gpu::Access::WriteOnly;
	}
	else
	{
		Desc.AccessMode = Gpu::Access::ReadWrite;
	}

	Image = ContextObject->GetContext()->CreateImage(Desc);
	return Image != nullptr;
}

bool UGPUImageObject::CreateImage3D(UGPUContextObject* ContextObject,
									int32 Width,
									int32 Height,
									int32 Depth,
									EGpuPixelFormat Format,
									bool bReadOnly,
									bool bWriteOnly)
{
	if (!ContextObject || !ContextObject->GetContext() || Width <= 0 || Height <= 0 || Depth <= 0)
	{
		return false;
	}

	Gpu::ImageDescription Desc;
	Desc.Type = Gpu::ImageType::Tex3D;
	Desc.Format = ToNativePixelFormat(Format);
	Desc.Width = static_cast<uint32>(Width);
	Desc.Height = static_cast<uint32>(Height);
	Desc.DepthOrLayers = static_cast<uint32>(Depth);

	if (bReadOnly)
	{
		Desc.AccessMode = Gpu::Access::ReadOnly;
	}
	else if (bWriteOnly)
	{
		Desc.AccessMode = Gpu::Access::WriteOnly;
	}
	else
	{
		Desc.AccessMode = Gpu::Access::ReadWrite;
	}

	Image = ContextObject->GetContext()->CreateImage(Desc);
	return Image != nullptr;
}

bool UGPUImageObject::IsValidImage() const
{
	return Image != nullptr;
}

int32 UGPUImageObject::GetWidth() const
{
	return Image ? static_cast<int32>(Image->GetWidth()) : 0;
}

int32 UGPUImageObject::GetHeight() const
{
	return Image ? static_cast<int32>(Image->GetHeight()) : 0;
}

int32 UGPUImageObject::GetDepthOrLayers() const
{
	return Image ? static_cast<int32>(Image->GetDepthOrLayers()) : 0;
}

EGpuImageType UGPUImageObject::GetImageType() const
{
	return Image ? FromNativeImageType(Image->GetType()) : EGpuImageType::Tex2D;
}

EGpuPixelFormat UGPUImageObject::GetPixelFormat() const
{
	return Image ? FromNativePixelFormat(Image->GetFormat()) : EGpuPixelFormat::Unknown;
}

bool UGPUImageObject::UploadBytes(UGPUContextObject* ContextObject,
								  const TArray<uint8>& Bytes)
{
	if (!ValidateContextAndQueue(ContextObject))
    {
        return false;
    }

    const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
    if (Bpp == 0)
    {
        return false;
    }

	Gpu::ImageRegion Region;
	Gpu::ImageLayout Layout;
    switch (Image->GetType())
    {
		case Gpu::ImageType::Tex2D:
		{
			Region = {0, 0, 0, Image->GetWidth(), Image->GetHeight(), 1};
			Layout.RowPitchBytes = Image->GetWidth() * Bpp;
			Layout.SlicePitchBytes = Image->GetWidth() * Image->GetHeight() * Bpp;
			break;
		}
		case Gpu::ImageType::Tex2DArray:
		case Gpu::ImageType::Tex3D:
		{
			Region = {0, 0, 0, Image->GetWidth(), Image->GetHeight(), Image->GetDepthOrLayers()};
			Layout.RowPitchBytes = Image->GetWidth() * Bpp;
			Layout.SlicePitchBytes = Image->GetWidth() * Image->GetHeight() * Bpp;
			break;
		}
		default:
			return false;
    }

    return Image->Upload(*ContextObject->GetDefaultQueue(),
						 Bytes.GetData(),
						 static_cast<size_t>(Bytes.Num()),
						 Region,
						 Layout);
}

bool UGPUImageObject::DownloadBytes(UGPUContextObject* ContextObject,
									TArray<uint8>& OutBytes)
{
	TArray64<uint8> Temp;
	if (!DownloadToCpuBytes(ContextObject, Temp))
	{
		return false;
	}

	OutBytes.SetNumUninitialized(static_cast<int32>(Temp.Num()));
	FMemory::Memcpy(OutBytes.GetData(), Temp.GetData(), Temp.Num());
	return true;
}

UTexture2D* UGPUImageObject::CreateTexture2D(UGPUContextObject* ContextObject,
											 bool bSRGB,
											 bool bGenerateMips)
{
	if (!ValidateContextAndQueue(ContextObject) || Image->GetType() != Gpu::ImageType::Tex2D)
	{
		return nullptr;
	}

	const EPixelFormat UEFormat = ToUEPixelFormat(Image->GetFormat());
	const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
	if (UEFormat == PF_Unknown)
	{
		return nullptr;
	}

	TArray64<uint8> BaseBytes;
	std::vector<RenderUtils::Mip> Mips;
	if (!DownloadAndBuildMips(ContextObject, bGenerateMips, BaseBytes, Mips))
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(static_cast<int32>(Image->GetWidth()),
													  static_cast<int32>(Image->GetHeight()),
													  UEFormat);

	if (!Texture)
	{
		FreeGeneratedMipChain(Mips);
		return nullptr;
	}

	Texture->NeverStream = true;
	Texture->SRGB = bSRGB;
	Texture->MipGenSettings = bGenerateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;

	Texture->SetPlatformData(new FTexturePlatformData());

	const bool bFilled = FillPlatformDataFromMips(Texture->GetPlatformData(),
												  UEFormat,
												  Mips,
												  Bpp,
												  Image->GetWidth(),
												  Image->GetHeight(),
												  1);

	FreeGeneratedMipChain(Mips);

	if (!bFilled)
	{
		return nullptr;
	}

	Texture->UpdateResource();
	return Texture;
}

UTextureRenderTarget2D* UGPUImageObject::CreateAndWriteRenderTarget2D(UGPUContextObject* ContextObject,
																	  FLinearColor ClearColor,
																	  bool bSRGB,
																	  bool bGenerateMips)
{
	if (!ValidateContextAndQueue(ContextObject) || Image->GetType() != Gpu::ImageType::Tex2D)
	{
		return nullptr;
	}

	const ETextureRenderTargetFormat UEFormat = ToUERenderTargetPixelFormat(Image->GetFormat());
	if (UEFormat == -1)
	{
		return nullptr;
	}

	TObjectPtr<UTextureRenderTarget2D> Texture = NewObject<UTextureRenderTarget2D>(GetTransientPackage(),
																				   NAME_None,
																				   RF_Transient);

	if (!Texture)
	{
		return nullptr;
	}

	Texture->RenderTargetFormat = UEFormat;
	Texture->ClearColor = ClearColor;
	Texture->bAutoGenerateMips = false;
	Texture->bCanCreateUAV = true;
	Texture->InitAutoFormat(static_cast<int32>(Image->GetWidth()), static_cast<int32>(Image->GetHeight()));
	Texture->UpdateResourceImmediate(true);

	UTexture2D* temp = CreateTexture2D(ContextObject, bSRGB, bGenerateMips);

	if (!RenderUtils::BlitTextureToRenderTarget_Immediate(temp, Texture))
	{
		return nullptr;
	}
	return Texture;
}

UTexture2DArray* UGPUImageObject::CreateTexture2DArray(UGPUContextObject* ContextObject,
													   bool bSRGB,
													   bool bGenerateMips)
{
	if (!ValidateContextAndQueue(ContextObject) || Image->GetType() != Gpu::ImageType::Tex2DArray)
    {
        return nullptr;
    }

	const EPixelFormat UEFormat = ToUEPixelFormat(Image->GetFormat());
	const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
    if (UEFormat == PF_Unknown)
    {
        return nullptr;
    }

	TArray64<uint8> BaseBytes;
	std::vector<RenderUtils::Mip> Mips;
	if (!DownloadAndBuildMips(ContextObject, bGenerateMips, BaseBytes, Mips))
	{
		return nullptr;
	}

    UTexture2DArray* Texture = NewObject<UTexture2DArray>(GetTransientPackage(), NAME_None, RF_Transient);
    if (!Texture)
    {
		FreeGeneratedMipChain(Mips);
        return nullptr;
    }

    Texture->NeverStream = true;
    Texture->SRGB = bSRGB;
	Texture->MipGenSettings = bGenerateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;

    Texture->SetPlatformData(new FTexturePlatformData());

	const bool bFilled = FillPlatformDataFromMips(Texture->GetPlatformData(),
												  UEFormat,
												  Mips,
												  Bpp,
												  Image->GetWidth(),
												  Image->GetHeight(),
												  Image->GetDepthOrLayers());

	FreeGeneratedMipChain(Mips);

	if (!bFilled)
	{
		return nullptr;
	}

	Texture->UpdateResource();
	return Texture;
}

UVolumeTexture* UGPUImageObject::CreateVolumeTexture(UGPUContextObject* ContextObject,
													 bool bSRGB,
													 bool bGenerateMips)
{
	if (!ValidateContextAndQueue(ContextObject) || Image->GetType() != Gpu::ImageType::Tex3D)
	{
		return nullptr;
	}

	const EPixelFormat UEFormat = ToUEPixelFormat(Image->GetFormat());
	const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
	if (UEFormat == PF_Unknown)
	{
		return nullptr;
	}

	TArray64<uint8> BaseBytes;
	std::vector<RenderUtils::Mip> Mips;
	if (!DownloadAndBuildMips(ContextObject, bGenerateMips, BaseBytes, Mips))
	{
		return nullptr;
	}

	UVolumeTexture* Texture = NewObject<UVolumeTexture>(GetTransientPackage(),
														NAME_None,
														RF_Transient);
	if (!Texture)
	{
		FreeGeneratedMipChain(Mips);
		return nullptr;
	}

	Texture->NeverStream = true;
	Texture->SRGB = bSRGB;
	Texture->MipGenSettings = bGenerateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;
	Texture->SetPlatformData(new FTexturePlatformData());

	const bool bFilled = FillPlatformDataFromMips(Texture->GetPlatformData(),
												  UEFormat,
												  Mips,
												  Bpp,
												  Image->GetWidth(),
												  Image->GetHeight(),
												  Image->GetDepthOrLayers());

	FreeGeneratedMipChain(Mips);

	if (!bFilled)
	{
		return nullptr;
	}

	Texture->UpdateResource();
	return Texture;
}

bool UGPUImageObject::WriteToRenderTarget2D(UGPUContextObject* ContextObject,
											UTextureRenderTarget2D* Output)
{
	bool isSRGB = Output->IsSRGB();
	UTexture2D* temp = CreateTexture2D(ContextObject, isSRGB, false);

	TObjectPtr<UTextureRenderTarget2D> Texture(Output);
	if (!RenderUtils::BlitTextureToRenderTarget_Immediate(temp, Texture))
	{
		return false;
	}
	return true;
}

bool UGPUImageObject::UpdateTexture2D(UGPUContextObject* ContextObject,
									  UTexture2D* Texture)
{
	if (!ValidateContextAndQueue(ContextObject) ||
		!Texture ||
		!Texture->GetPlatformData() ||
		Image->GetType() != Gpu::ImageType::Tex2D)
	{
		return false;
	}

	const bool generateMips = Texture->MipGenSettings != TMGS_NoMipmaps;
	if (UpdateTexture_Internal(Texture->GetPlatformData(), ContextObject, generateMips))
	{
		Texture->UpdateResource();
		return true;
	}
	return false;
}

bool UGPUImageObject::UpdateTexture2DArray(UGPUContextObject* ContextObject,
										   UTexture2DArray* Texture)
{
	if (!ValidateContextAndQueue(ContextObject) ||
		!Texture ||
		!Texture->GetPlatformData() ||
		Image->GetType() != Gpu::ImageType::Tex2DArray)
	{
		return false;
	}

	const bool generateMips = Texture->MipGenSettings != TMGS_NoMipmaps;
	if (UpdateTexture_Internal(Texture->GetPlatformData(), ContextObject, generateMips))
	{
		Texture->UpdateResource();
		return true;
	}
	return false;
}

bool UGPUImageObject::UpdateVolumeTexture(UGPUContextObject* ContextObject,
										  UVolumeTexture* Texture)
{
	if (!ValidateContextAndQueue(ContextObject) ||
		!Texture ||
		!Texture->GetPlatformData() ||
		Image->GetType() != Gpu::ImageType::Tex3D)
	{
		return false;
	}

	const bool generateMips = Texture->MipGenSettings != TMGS_NoMipmaps;
	if (UpdateTexture_Internal(Texture->GetPlatformData(), ContextObject, generateMips))
	{
		Texture->UpdateResource();
		return true;
	}
	return false;
}

Gpu::PixelFormat UGPUImageObject::ToNativePixelFormat(EGpuPixelFormat Format)
{
	switch (Format)
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

EGpuPixelFormat UGPUImageObject::FromNativePixelFormat(Gpu::PixelFormat Format)
{
	switch (Format)
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

EGpuImageType UGPUImageObject::FromNativeImageType(Gpu::ImageType Type)
{
	switch (Type)
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

EPixelFormat UGPUImageObject::ToUEPixelFormat(Gpu::PixelFormat Format)
{
	switch (Format)
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

ETextureRenderTargetFormat UGPUImageObject::ToUERenderTargetPixelFormat(Gpu::PixelFormat Format)
{
	switch (Format)
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

size_t UGPUImageObject::GetBytesPerPixel(Gpu::PixelFormat Format)
{
	switch (Format)
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

bool UGPUImageObject::ValidateContextAndQueue(UGPUContextObject* ContextObject) const
{
	return ContextObject &&
           ContextObject->GetContext() &&
           ContextObject->GetDefaultQueue() &&
           Image != nullptr;
}

uint32 UGPUImageObject::GetChannelCount(Gpu::PixelFormat Format)
{
	switch (Format)
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

bool UGPUImageObject::GenerateMipChain(Gpu::PixelFormat Format,
									   uint8_t* SourceBytes,
									   uint32_t Width,
									   uint32_t Height,
									   uint32_t Layers,
									   std::vector<RenderUtils::Mip>& OutMips)
{
	const uint32 Channels = GetChannelCount(Format);
	if (!SourceBytes || Channels == 0)
	{
		return false;
	}

	switch (Format)
	{
		case Gpu::PixelFormat::R8:
		case Gpu::PixelFormat::RG8:
		case Gpu::PixelFormat::RGBA8:
		{
			RenderUtils::GenerateMipsInt8(OutMips,
										  reinterpret_cast<uint8_t*>(SourceBytes),
										  Width,
										  Height,
										  Layers,
										  Channels);
			return true;
		}
		case Gpu::PixelFormat::R16F:
		case Gpu::PixelFormat::RG16F:
		case Gpu::PixelFormat::RGBA16F:
		{
			RenderUtils::GenerateMipsFloat16(OutMips,
											 reinterpret_cast<FFloat16*>(SourceBytes),
											 Width,
											 Height,
											 Layers,
											 Channels);
			return true;
		}
		case Gpu::PixelFormat::R32F:
		case Gpu::PixelFormat::RG32F:
		case Gpu::PixelFormat::RGBA32F:
		{
			RenderUtils::GenerateMipsFloat(OutMips,
										   reinterpret_cast<float*>(SourceBytes),
										   Width,
										   Height,
										   Layers,
										   Channels);
			return true;
		}
		case Gpu::PixelFormat::R32U:
		case Gpu::PixelFormat::RG32U:
		case Gpu::PixelFormat::RGBA32U:
		{
			RenderUtils::GenerateMipsUInt32(OutMips,
											reinterpret_cast<uint32_t*>(SourceBytes),
											Width,
											Height,
											Layers,
											Channels);
			return true;
		}
		case Gpu::PixelFormat::R32S:
		{
			RenderUtils::GenerateMipsInt32(OutMips,
										   reinterpret_cast<int32_t*>(SourceBytes),
										   Width,
										   Height,
										   Layers,
										   Channels);
			return true;
		}
		default:
			return false;
	}
}

void UGPUImageObject::FreeGeneratedMipChain(std::vector<RenderUtils::Mip>& Mips)
{
	// Mip 0 points into the downloaded CPU byte array.
	// Mips 1+ are allocated with new[] inside GenerateMip.
	for (size_t i = 1; i < Mips.size(); ++i)
	{
		delete[] Mips[i].mPixels;
		Mips[i].mPixels = nullptr;
	}

	Mips.clear();
}

bool UGPUImageObject::FillPlatformDataFromMips(FTexturePlatformData* PlatformData,
											   EPixelFormat UEFormat,
											   const std::vector<RenderUtils::Mip>& Mips,
											   size_t BytesPerPixel,
											   uint32_t BaseWidth,
											   uint32_t BaseHeight,
											   uint32_t Layers)
{
	if (!PlatformData || Mips.empty() || BytesPerPixel == 0)
	{
		return false;
	}

	PlatformData->SizeX = BaseWidth;
	PlatformData->SizeY = BaseHeight;
	PlatformData->SetNumSlices(Layers);
	PlatformData->PixelFormat = UEFormat;
	PlatformData->Mips.Empty();

	for (const RenderUtils::Mip& SrcMip : Mips)
	{
		FTexture2DMipMap* DstMip = new FTexture2DMipMap();
		PlatformData->Mips.Add(DstMip);

		DstMip->SizeX = static_cast<int32>(SrcMip.mWidth);
		DstMip->SizeY = static_cast<int32>(SrcMip.mHeight);
		DstMip->SizeZ = static_cast<int32>(SrcMip.mSlices);

		const int64 MipBytes = static_cast<int64>(SrcMip.mWidth) *
							   static_cast<int64>(SrcMip.mHeight) *
							   static_cast<int64>(SrcMip.mSlices) *
							   static_cast<int64>(BytesPerPixel);

		DstMip->BulkData.Lock(LOCK_READ_WRITE);
		void* Dst = DstMip->BulkData.Realloc(MipBytes);
		FMemory::Memcpy(Dst, SrcMip.mPixels, MipBytes);
		DstMip->BulkData.Unlock();
	}

	return true;
}

bool UGPUImageObject::DownloadAndBuildMips(UGPUContextObject* ContextObject,
										   bool bGenerateMips,
										   TArray64<uint8_t>& OutBaseBytes,
										   std::vector<RenderUtils::Mip>& OutMips)
{
	if (!DownloadToCpuBytes(ContextObject, OutBaseBytes))
	{
		return false;
	}

	const uint32 Width = Image->GetWidth();
	const uint32 Height = Image->GetHeight();
	const uint32 Layers = Image->GetType() == Gpu::ImageType::Tex2D ? 1 : Image->GetDepthOrLayers();

	if (bGenerateMips)
	{
		return GenerateMipChain(Image->GetFormat(),
								OutBaseBytes.GetData(),
								Width,
								Height,
								Layers,
								OutMips);
	}

	const uint32 Channels = GetChannelCount(Image->GetFormat());

	RenderUtils::Mip BaseMip;
	BaseMip.mWidth = Width;
	BaseMip.mHeight = Height;
	BaseMip.mSlices = Layers;
	BaseMip.mChannels = Channels;
	BaseMip.mPixels = OutBaseBytes.GetData();

	OutMips.clear();
	OutMips.push_back(BaseMip);
	return true;
}

bool UGPUImageObject::DownloadToCpuBytes(UGPUContextObject* ContextObject,
										 TArray64<uint8>& OutBytes) const
{
	if (!ValidateContextAndQueue(ContextObject))
    {
        return false;
    }

    const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
    if (Bpp == 0)
    {
        return false;
    }

	Gpu::ImageRegion Region;
    size_t TotalBytes = 0;

    switch (Image->GetType())
    {
		case Gpu::ImageType::Tex2D:
		{
		    Region = {0, 0, 0, Image->GetWidth(), Image->GetHeight(), 1};
		    TotalBytes = static_cast<size_t>(Image->GetWidth()) * Image->GetHeight() * Bpp;
		    break;
		}
		case Gpu::ImageType::Tex2DArray:
		case Gpu::ImageType::Tex3D:
		{
		    Region = {0, 0, 0, Image->GetWidth(), Image->GetHeight(), Image->GetDepthOrLayers()};
		    TotalBytes = static_cast<size_t>(Image->GetWidth()) * Image->GetHeight() * Image->GetDepthOrLayers() * Bpp;
		    break;
		}
		default:
			return false;
    }

    OutBytes.SetNumUninitialized(TotalBytes);

    return Image->Download(*ContextObject->GetDefaultQueue(),
						   OutBytes.GetData(),
						   TotalBytes,
						   Region);
}

bool UGPUImageObject::UpdateTexture_Internal(FTexturePlatformData* PlatformData,
											 UGPUContextObject* ContextObject,
											 bool bGenerateMips)
{
	TArray64<uint8> BaseBytes;
	std::vector<RenderUtils::Mip> Mips;
	if (!DownloadAndBuildMips(ContextObject, bGenerateMips, BaseBytes, Mips))
	{
		return false;
	}

	if (PlatformData->Mips.Num() != static_cast<int32>(Mips.size()))
	{
		FreeGeneratedMipChain(Mips);
		return false;
	}

	const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
	for (int32 MipIndex = 0; MipIndex < PlatformData->Mips.Num(); ++MipIndex)
	{
		const RenderUtils::Mip& SrcMip = Mips[MipIndex];
		FTexture2DMipMap& DstMip = PlatformData->Mips[MipIndex];

		const int64 MipBytes = static_cast<int64>(SrcMip.mWidth) *
							   static_cast<int64>(SrcMip.mHeight) *
							   static_cast<int64>(SrcMip.mSlices) *
							   static_cast<int64>(Bpp);

		void* DstData = DstMip.BulkData.Lock(LOCK_READ_WRITE);
		if (!DstData)
		{
			FreeGeneratedMipChain(Mips);
			return false;
		}

		if (DstMip.BulkData.GetBulkDataSize() != MipBytes)
		{
			DstMip.BulkData.Unlock();
			FreeGeneratedMipChain(Mips);
			return false;
		}

		FMemory::Memcpy(DstData, SrcMip.mPixels, MipBytes);
		DstMip.BulkData.Unlock();
	}

	FreeGeneratedMipChain(Mips);
	return true;
}