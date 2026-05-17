#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTarget2D.h"

#include "GPU/GPUContext.h"
#include "GPU/GPUImage.h"
#include "GPU/GPUQueue.h"

#include "GPUImageObject.generated.h"

UENUM(BlueprintType)
enum class EGpuImageType : uint8
{
    Tex2D,
    Tex2DArray,
    Tex3D
};

UENUM(BlueprintType)
enum class EGpuPixelFormat : uint8
{
    Unknown,
    R8,
    RG8,
    RGBA8,
    R16F,
    RG16F,
    RGBA16F,
    R32F,
    RG32F,
    RGBA32F,
    R32U,
    RG32U,
    RGBA32U,
    R32S
};

UCLASS(BlueprintType)
class GPUWORKS_API UGPUImageObject : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool CreateImage2D(UGPUContextObject* ContextObject,
                       int32 Width,
                       int32 Height,
                       EGpuPixelFormat Format,
                       bool bReadOnly = false,
                       bool bWriteOnly = false);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool CreateImage2DArray(UGPUContextObject* ContextObject,
                            int32 Width,
                            int32 Height,
                            int32 Layers,
                            EGpuPixelFormat Format,
                            bool bReadOnly = false,
                            bool bWriteOnly = false);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool CreateImage3D(UGPUContextObject* ContextObject,
                       int32 Width,
                       int32 Height,
                       int32 Depth,
                       EGpuPixelFormat Format,
                       bool bReadOnly = false,
                       bool bWriteOnly = false);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool IsValidImage() const;

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    int32 GetWidth() const;

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    int32 GetHeight() const;

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    int32 GetDepthOrLayers() const;

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    EGpuImageType GetImageType() const;

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    EGpuPixelFormat GetPixelFormat() const;

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UploadBytes(UGPUContextObject* ContextObject,
                     const TArray<uint8>& Bytes);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool DownloadBytes(UGPUContextObject* ContextObject,
                       TArray<uint8>& OutBytes);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UTexture2D* CreateTexture2D(UGPUContextObject* ContextObject,
                                bool bSRGB = false,
                                bool bGenerateMips = false);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UTextureRenderTarget2D* CreateAndWriteRenderTarget2D(UGPUContextObject* ContextObject,
                                                         FLinearColor ClearColor = FLinearColor::Transparent,
                                                         bool bSRGB = false,
                                                         bool bGenerateMips = false);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UTexture2DArray* CreateTexture2DArray(UGPUContextObject* ContextObject,
                                          bool bSRGB = false,
                                          bool bGenerateMips = false);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UVolumeTexture* CreateVolumeTexture(UGPUContextObject* ContextObject,
                                        bool bSRGB = false,
                                        bool bGenerateMips = false);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool WriteToRenderTarget2D(UGPUContextObject* ContextObject,
                               UTextureRenderTarget2D* Output);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UpdateTexture2D(UGPUContextObject* ContextObject,
                         UTexture2D* Texture);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UpdateTexture2DArray(UGPUContextObject* ContextObject,
                              UTexture2DArray* Texture);

    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UpdateVolumeTexture(UGPUContextObject* ContextObject,
                             UVolumeTexture* Texture);

    std::shared_ptr<Gpu::IImage> GetImage() const { return Image; }
private:
    static Gpu::PixelFormat ToNativePixelFormat(EGpuPixelFormat Format);

    static EGpuPixelFormat FromNativePixelFormat(Gpu::PixelFormat Format);

    static EGpuImageType FromNativeImageType(Gpu::ImageType Type);

    static EPixelFormat ToUEPixelFormat(Gpu::PixelFormat Format);
    static ETextureRenderTargetFormat ToUERenderTargetPixelFormat(Gpu::PixelFormat Format);

    static size_t GetBytesPerPixel(Gpu::PixelFormat Format);

    bool ValidateContextAndQueue(UGPUContextObject* ContextObject) const;

    bool DownloadToCpuBytes(UGPUContextObject* ContextObject,
                            TArray64<uint8>& OutBytes) const;

    bool UploadToTexture2D_Internal(UGPUContextObject* ContextObject,
                                    UTexture2D* Texture);

    bool UploadToTexture2DArray_Internal(UGPUContextObject* ContextObject,
                                         UTexture2DArray* Texture);

    bool UploadToVolumeTexture_Internal(UGPUContextObject* ContextObject,
                                        UVolumeTexture* Texture);
private:
    std::shared_ptr<Gpu::IImage> Image;
};