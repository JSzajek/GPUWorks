#include "Interops/UE/GPUImageObject.h"
#include "Interops/UE/GPUContextObject.h"

#include "Engine/Texture.h"
#include "Render/UTextureUtils.h"

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
	if (UEFormat == PF_Unknown)
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(static_cast<int32>(Image->GetWidth()),
													  static_cast<int32>(Image->GetHeight()),
													  UEFormat);

	if (!Texture)
	{
		return nullptr;
	}

	Texture->SRGB = bSRGB;
	Texture->MipGenSettings = bGenerateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;
	Texture->UpdateResource();

	if (!UploadToTexture2D_Internal(ContextObject, Texture))
	{
		return nullptr;
	}
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

	if (!UTextureUtils::BlitTextureToRenderTarget_Immediate(temp, Texture))
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
    if (UEFormat == PF_Unknown)
    {
        return nullptr;
    }

    UTexture2DArray* Texture = NewObject<UTexture2DArray>(GetTransientPackage(), NAME_None, RF_Transient);
    if (!Texture)
    {
        return nullptr;
    }

    Texture->NeverStream = true;
    Texture->SRGB = bSRGB;
    Texture->MipGenSettings = bGenerateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;

    Texture->SetPlatformData(new FTexturePlatformData());

	FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	PlatformData->SizeX = Image->GetWidth();
	PlatformData->SizeY = Image->GetHeight();
	PlatformData->SetNumSlices(Image->GetDepthOrLayers());
	PlatformData->PixelFormat = UEFormat;

    FTexture2DMipMap* Mip = new FTexture2DMipMap();
    PlatformData->Mips.Add(Mip);

    const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
    const int64 TotalBytes = static_cast<int64>(Image->GetWidth()) *
							 static_cast<int64>(Image->GetHeight()) *
							 static_cast<int64>(Image->GetDepthOrLayers()) *
							 static_cast<int64>(Bpp);

    Mip->SizeX = Image->GetWidth();
    Mip->SizeY = Image->GetHeight();
    Mip->SizeZ = Image->GetDepthOrLayers();
    Mip->BulkData.Lock(LOCK_READ_WRITE);
    void* Ptr = Mip->BulkData.Realloc(TotalBytes);
    FMemory::Memzero(Ptr, TotalBytes);
    Mip->BulkData.Unlock();

    Texture->UpdateResource();

    if (!UploadToTexture2DArray_Internal(ContextObject, Texture))
    {
        return nullptr;
    }
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
	if (UEFormat == PF_Unknown)
	{
		return nullptr;
	}

	UVolumeTexture* Texture = NewObject<UVolumeTexture>(GetTransientPackage(),
														NAME_None,
														RF_Transient);
	if (!Texture)
	{
		return nullptr;
	}

	Texture->NeverStream = true;
	Texture->SRGB = bSRGB;
	Texture->MipGenSettings = bGenerateMips ? TMGS_FromTextureGroup : TMGS_NoMipmaps;

	Texture->SetPlatformData(new FTexturePlatformData());

	FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	PlatformData->SizeX = Image->GetWidth();
	PlatformData->SizeY = Image->GetHeight();
	PlatformData->SetNumSlices(Image->GetDepthOrLayers());
	PlatformData->PixelFormat = UEFormat;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	PlatformData->Mips.Add(Mip);

	const size_t Bpp = GetBytesPerPixel(Image->GetFormat());
	const int64 TotalBytes = static_cast<int64>(Image->GetWidth()) *
							 static_cast<int64>(Image->GetHeight()) *
							 static_cast<int64>(Image->GetDepthOrLayers()) *
							 static_cast<int64>(Bpp);

	Mip->SizeX = Image->GetWidth();
	Mip->SizeY = Image->GetHeight();
	Mip->SizeZ = Image->GetDepthOrLayers();
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* Ptr = Mip->BulkData.Realloc(TotalBytes);
	FMemory::Memzero(Ptr, TotalBytes);
	Mip->BulkData.Unlock();

	Texture->UpdateResource();

	UploadToVolumeTexture_Internal(ContextObject, Texture);
	return Texture;
}

bool UGPUImageObject::WriteToRenderTarget2D(UGPUContextObject* ContextObject,
											UTextureRenderTarget2D* Output)
{
	bool isSRGB = Output->IsSRGB();
	UTexture2D* temp = CreateTexture2D(ContextObject, isSRGB, false);

	TObjectPtr<UTextureRenderTarget2D> Texture(Output);
	if (!UTextureUtils::BlitTextureToRenderTarget_Immediate(temp, Texture))
	{
		return false;
	}
	return true;
}

bool UGPUImageObject::UpdateTexture2D(UGPUContextObject* ContextObject,
									  UTexture2D* Texture)
{
	return UploadToTexture2D_Internal(ContextObject, Texture);
}

bool UGPUImageObject::UpdateTexture2DArray(UGPUContextObject* ContextObject,
										   UTexture2DArray* Texture)
{
	return UploadToTexture2DArray_Internal(ContextObject, Texture);
}

bool UGPUImageObject::UpdateVolumeTexture(UGPUContextObject* ContextObject,
										  UVolumeTexture* Texture)
{
	return UploadToVolumeTexture_Internal(ContextObject, Texture);
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
	Gpu::ImageLayout Layout;
    size_t TotalBytes = 0;

    switch (Image->GetType())
    {
		case Gpu::ImageType::Tex2D:
		{
		    Region = {0, 0, 0, Image->GetWidth(), Image->GetHeight(), 1};
		    Layout.RowPitchBytes = Image->GetWidth() * Bpp;
		    Layout.SlicePitchBytes = Image->GetWidth() * Image->GetHeight() * Bpp;
		    TotalBytes = static_cast<size_t>(Image->GetWidth()) * Image->GetHeight() * Bpp;
		    break;
		}
		case Gpu::ImageType::Tex2DArray:
		case Gpu::ImageType::Tex3D:
		{
		    Region = {0, 0, 0, Image->GetWidth(), Image->GetHeight(), Image->GetDepthOrLayers()};
		    Layout.RowPitchBytes = Image->GetWidth() * Bpp;
		    Layout.SlicePitchBytes = Image->GetWidth() * Image->GetHeight() * Bpp;
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

bool UGPUImageObject::UploadToTexture2D_Internal(UGPUContextObject* ContextObject,
												 UTexture2D* Texture)
{
	if (!ValidateContextAndQueue(ContextObject) ||
		!Texture ||
		!Texture->GetPlatformData() ||
		Texture->GetPlatformData()->Mips.Num() == 0 ||
		Image->GetType() != Gpu::ImageType::Tex2D)
	{
		return false;
	}

	TArray64<uint8> CpuBytes;
	if (!DownloadToCpuBytes(ContextObject, CpuBytes))
	{
		return false;
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, CpuBytes.GetData(), CpuBytes.Num());
	Mip.BulkData.Unlock();

	Texture->UpdateResource();
	return true;
}

bool UGPUImageObject::UploadToTexture2DArray_Internal(UGPUContextObject* ContextObject,
													  UTexture2DArray* Texture)
{
	if (!ValidateContextAndQueue(ContextObject) ||
		!Texture ||
		!Texture->GetPlatformData() ||
		Texture->GetPlatformData()->Mips.Num() == 0 ||
		Image->GetType() != Gpu::ImageType::Tex2DArray)
	{
		return false;
	}

	TArray64<uint8> CpuBytes;
	if (!DownloadToCpuBytes(ContextObject, CpuBytes))
	{
		return false;
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, CpuBytes.GetData(), CpuBytes.Num());
	Mip.BulkData.Unlock();

	Texture->UpdateResource();
	return true;
}

bool UGPUImageObject::UploadToVolumeTexture_Internal(UGPUContextObject* ContextObject,
													 UVolumeTexture* Texture)
{
	if (!ValidateContextAndQueue(ContextObject) ||
		!Texture ||
		!Texture->GetPlatformData() ||
		Texture->GetPlatformData()->Mips.Num() == 0 ||
		Image->GetType() != Gpu::ImageType::Tex3D)
	{
		return false;
	}

	TArray64<uint8> CpuBytes;
	if (!DownloadToCpuBytes(ContextObject, CpuBytes))
	{
		return false;
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, CpuBytes.GetData(), CpuBytes.Num());
	Mip.BulkData.Unlock();

	Texture->UpdateResource();
	return true;
}