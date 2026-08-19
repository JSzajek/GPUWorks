#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Render/UTextureUtils.h"

#include "GPU/GPUTypes.h"
#include "GPU/GPUImage.h"

#include "GPUImageObject.generated.h"

/// <summary>
/// Enum representing the type of GPU image resource.
/// </summary>
UENUM(BlueprintType)
enum class EGpuImageType : uint8
{
    Tex2D,
    Tex2DArray,
    Tex3D
};

/// <summary>
/// Enum representing the pixel format of a GPU image resource.
/// </summary>
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

/// <summary>
/// GPU image wrapper for Unreal Engine. This class manages a GPU image resource 
/// and provides methods for creating, uploading, downloading, and converting to Unreal textures.
/// </summary>
UCLASS(BlueprintType)
class GPUWORKS_API UGPUImageObject : public UObject
{
    GENERATED_BODY()
public:
    /// <summary>
	/// Creates a 2D GPU image resource with the specified width, 
    /// height, pixel format, and access flags.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="width">The width of the image</param>
    /// <param name="height">The height of the image</param>
    /// <param name="format">The pixel format of the image</param>
    /// <param name="readOnly">Whether the image is read-only</param>
    /// <param name="writeOnly">Whether the image is write-only</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool CreateImage2D(UGPUContextObject* contextObject,
                       int32 width,
                       int32 height,
                       EGpuPixelFormat format,
                       bool readOnly = false,
                       bool writeOnly = false);

    /// <summary>
	/// Creates a 2D array GPU image resource with the specified width, 
    /// height, number of layers, pixel format, and access flags.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="width">The width of the image</param>
    /// <param name="height">The height of the image</param>
    /// <param name="layers">The number of layers in the image</param>
    /// <param name="format">The pixel format of the image</param>
    /// <param name="readOnly">Whether the image is read-only</param>
    /// <param name="writeOnly">Whether the image is write-only</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool CreateImage2DArray(UGPUContextObject* contextObject,
                            int32 width,
                            int32 height,
                            int32 layers,
                            EGpuPixelFormat format,
                            bool readOnly = false,
                            bool writeOnly = false);  
                   
    /// <summary>
	/// Creates a 3D GPU image resource with the specified width, 
    /// height, depth, pixel format, and access flags.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="width">The width of the image</param>
    /// <param name="height">The height of the image</param>
    /// <param name="depth">The depth of the image</param>
    /// <param name="format">The pixel format of the image</param>
    /// <param name="readOnly">Whether the image is read-only</param>
    /// <param name="bWriteOnly">Whether the image is write-only</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool CreateImage3D(UGPUContextObject* contextObject,
                       int32 width,
                       int32 height,
                       int32 depth,
                       EGpuPixelFormat format,
                       bool readOnly = false,
                       bool bWriteOnly = false);

    /// <summary>
	/// Checks whether the GPU image resource is valid and properly initialized.
    /// </summary>
    /// <returns>True if the image is valid</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool IsValidImage() const;

    /// <summary>
    /// Retrieves the width of the GPU image.
    /// </summary>
    /// <returns>The width of the image</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    int32 GetWidth() const;

    /// <summary>
    /// Retrieves the height of the GPU image.
    /// </summary>
    /// <returns>The height of the image</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    int32 GetHeight() const;

    /// <summary>
	/// Retrieves the depth of GPU image. 
    /// 
    /// For 2D images, this will return 1.
    /// For 2D array images, this will return the number of layers.
    /// For 3D images, this will return the number of slices/depth.
    /// </summary>
    /// <returns>The depth or layer of the image</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    int32 GetDepthOrLayers() const;

    /// <summary>
	/// Retrieves the type of the GPU image resource.
    /// </summary>
    /// <returns>The GPU image type</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    EGpuImageType GetImageType() const;

    /// <summary>
	/// Retrieves the pixel format of the GPU image resource.
    /// </summary>
    /// <returns>The GPU pixel format</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    EGpuPixelFormat GetPixelFormat() const;

    /// <summary>
	/// Uploads byte data to the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="bytes">The byte data to upload</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UploadBytes(UGPUContextObject* contextObject,
                     const TArray<uint8>& bytes);

    /// <summary>
    /// Downloads the byte data from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="bytes">The downloaded byte data</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool DownloadBytes(UGPUContextObject* contextObject,
                       TArray<uint8>& bytes);

    /// <summary>
    /// Fills the GPU image resource with the passed color.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="color">The fill color</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool FillColor(UGPUContextObject* contextObject,
                   const FColor& color);

    /// <summary>
	/// Create a UTexture2D from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="isSRGB">Whether the texture should use sRGB color space</param>
    /// <param name="generateMips">Whether to generate mipmaps for the texture</param>
    /// <returns>The created UTexture2D object</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UTexture2D* CreateTexture2D(UGPUContextObject* contextObject,
                                bool isSRGB = false,
                                bool generateMips = false);

    /// <summary>
	/// Creates and writes to a UTextureRenderTarget2D from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="clearColor">The clear color for the render target</param>
    /// <param name="isSRGB">Whether the render target should use sRGB color space</param>
    /// <param name="generateMips">Whether to generate mipmaps for the render target</param>
    /// <returns>The created UTextureRenderTarget2D object</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UTextureRenderTarget2D* CreateAndWriteRenderTarget2D(UGPUContextObject* contextObject,
                                                         FLinearColor clearColor = FLinearColor::Transparent,
                                                         bool isSRGB = false,
                                                         bool generateMips = false);

    /// <summary>
	/// Creates a UTexture2DArray from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="isSRGB">Whether the texture should use sRGB color space</param>
    /// <param name="generateMips">Whether to generate mipmaps for the texture</param>
    /// <returns>The created UTexture2DArray object</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UTexture2DArray* CreateTexture2DArray(UGPUContextObject* contextObject,
                                          bool isSRGB = false,
                                          bool generateMips = false);

    /// <summary>
	/// Creates a UVolumeTexture from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="isSRGB">Whether the texture should use sRGB color space</param>
    /// <param name="generateMips">Whether to generate mipmaps for the texture</param>
    /// <returns>The created UVolumeTexture object</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    UVolumeTexture* CreateVolumeTexture(UGPUContextObject* contextObject,
                                        bool isSRGB = false,
                                        bool generateMips = false);

    /// <summary>
	/// Writes the GPU image resource to a UTextureRenderTarget2D.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="output">The output render target</param>
    /// <returns>True if the operation was successful, false otherwise</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool WriteToRenderTarget2D(UGPUContextObject* contextObject,
                               UTextureRenderTarget2D* output);

    /// <summary>
	/// Updates the contents of a UTexture2D with the data from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="texture">The texture to update</param>
    /// <returns>True if the operation was successful, false otherwise</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UpdateTexture2D(UGPUContextObject* contextObject,
                         UTexture2D* texture);

    /// <summary>
	/// Asynchronously updates the contents of a UTexture2D with the data from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="texture">The texture to update</param>
    /// <param name="completionCallback">The callback function to be called upon completion</param>
    /// <param name="callbackThread">The thread on which to execute the callback</param>
    /// <returns>True if the operation was successfully initiated, false otherwise</returns>
    bool UpdateTexture2DAsync(UGPUContextObject* contextObject,
                              UTexture2D* texture,
                              const std::function<void(bool)>& completionCallback,
                              ENamedThreads::Type callbackThread = ENamedThreads::GameThread);

    /// <summary>
	/// Updates the contents of a UTexture2DArray with the data from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="texture">The texture to update</param>
    /// <returns>True if the operation was successful, false otherwise</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UpdateTexture2DArray(UGPUContextObject* contextObject,
                              UTexture2DArray* texture);

    /// <summary>
	/// Updates the contents of a UVolumeTexture with the data from the GPU image resource.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="texture">The texture to update</param>
    /// <returns>True if the operation was successful, false otherwise</returns>
    UFUNCTION(BlueprintCallable, Category="GPU|Image")
    bool UpdateVolumeTexture(UGPUContextObject* contextObject,
                             UVolumeTexture* texture);

    /// <summary>
	/// Retrieves the underlying GPU image resource.
    /// </summary>
    /// <returns>The GPU image resource</returns>
    std::shared_ptr<Gpu::IImage> GetImage() const { return mpImage; }
private:
    /// <summary>
	/// Converts the EGpuPixelFormat enum to the native Gpu::PixelFormat enum.
    /// </summary>
    /// <param name="format">The GPU pixel format</param>
    /// <returns>The native GPU pixel format</returns>
    Gpu::PixelFormat ToNativePixelFormat(EGpuPixelFormat format) const;

    /// <summary>
	/// Convert the native Gpu::PixelFormat enum to the EGpuPixelFormat enum.
    /// </summary>
    /// <param name="format">The native GPU pixel format</param>
    /// <returns>The corresponding EGpuPixelFormat value</returns>
    EGpuPixelFormat FromNativePixelFormat(Gpu::PixelFormat format) const;

    /// <summary>
	/// Converts the native Gpu::ImageType enum to the EGpuImageType enum.
    /// </summary>
    /// <param name="type">The native GPU image type</param>
    /// <returns>The corresponding EGpuImageType value</returns>
    EGpuImageType FromNativeImageType(Gpu::ImageType type) const;

    /// <summary>
	/// Converts the Gpu::PixelFormat to Unreal Engine's EPixelFormat.
    /// </summary>
    /// <param name="format">The native GPU pixel format</param>
    /// <returns>The corresponding Unreal Engine pixel format</returns>
    EPixelFormat ToUEPixelFormat(Gpu::PixelFormat format) const;

    /// <summary>
	/// Converts the Gpu::PixelFormat to Unreal Engine's ETextureRenderTargetFormat.
    /// </summary>
    /// <param name="format">The native GPU pixel format</param>
    /// <returns>The corresponding Unreal Engine texture render target format</returns>
    ETextureRenderTargetFormat ToUERenderTargetPixelFormat(Gpu::PixelFormat format) const;

    /// <summary>
	/// Retrieves the number of bytes per pixel for the specified Gpu::PixelFormat.
    /// </summary>
    /// <param name="format">The native GPU pixel format</param>
    /// <returns>The number of bytes per pixel</returns>
    size_t GetBytesPerPixel(Gpu::PixelFormat format) const;

    /// <summary>
	/// Validates the GPU context and queue for the image operations.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <returns>True if the context and queue are valid, false otherwise</returns>
    bool ValidateContextAndQueue(UGPUContextObject* contextObject) const;

    /// <summary>
	/// Retrieves the number of channels for the specified Gpu::PixelFormat.
    /// </summary>
    /// <param name="format">The native GPU pixel format</param>
    /// <returns>The number of channels</returns>
    uint32 GetChannelCount(Gpu::PixelFormat format) const;

    /// <summary>
	/// Helper function to generate a mipmap chain from the source image data.
    /// </summary>
    /// <param name="format">The native GPU pixel format</param>
    /// <param name="sourceBytes">The source image data</param>
    /// <param name="width">The width of the source image</param>
    /// <param name="height">The height of the source image</param>
    /// <param name="layers">The number of layers in the source image</param>
    /// <param name="mips">The vector to store the generated mipmaps</param>
    /// <returns>True if the mipmap chain was successfully generated, false otherwise</returns>
    bool GenerateMipChain(Gpu::PixelFormat format,
                          uint8_t* sourceBytes,
                          uint32_t width,
                          uint32_t height,
                          uint32_t layers,
                          std::vector<RenderUtils::Mip>& mips);

    /// <summary>
	/// Frees the memory allocated for the generated mipmap chain.
    /// </summary>
    /// <param name="mips">The vector containing the generated mipmaps</param>
    void FreeGeneratedMipChain(std::vector<RenderUtils::Mip>& mips);

    /// <summary>
	/// Fills the platform data structure with the provided mipmap data, pixel format, and dimensions.
    /// </summary>
    /// <param name="platformData">The platform data structure to fill</param>
    /// <param name="format">The native GPU pixel format</param>
    /// <param name="mips">The vector containing the generated mipmaps</param>
    /// <param name="bytesPerPixel">The number of bytes per pixel</param>
    /// <param name="baseWidth">The width of the base mip level</param>
    /// <param name="baseHeight">The height of the base mip level</param>
    /// <param name="layers">The number of layers in the source image</param>
    /// <returns>True if the platform data was successfully filled, false otherwise</returns>
    bool FillPlatformDataFromMips(FTexturePlatformData* platformData,
                                  EPixelFormat format,
                                  const std::vector<RenderUtils::Mip>& mips,
                                  size_t bytesPerPixel,
                                  uint32_t baseWidth,
                                  uint32_t baseHeight,
                                  uint32_t layers);

    /// <summary>
	/// Helper function to download the base image data and generate mipmaps if requested.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="generateMips">Whether to generate mipmaps</param>
    /// <param name="outBaseBytes">The array to store the base image data</param>
    /// <param name="outMips">The vector to store the generated mipmaps</param>
    /// <returns>True if the operation was successful, false otherwise</returns>
    bool DownloadAndBuildMips(UGPUContextObject* contextObject,
                              bool generateMips,
                              TArray64<uint8_t>& outBaseBytes,
                              std::vector<RenderUtils::Mip>& outMips);

    /// <summary>
	/// Downloads the GPU image resource to CPU memory as a byte array.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="outBytes">The array to store the downloaded image data</param>
    /// <returns>True if the operation was successful, false otherwise</returns>
    bool DownloadToCpuBytes(UGPUContextObject* contextObject,
                            TArray64<uint8>& outBytes) const;

    /// <summary>
	/// Update the contents of a UTexture2D with the data from the GPU image resource.
    /// </summary>
    /// <param name="platformData">The platform data structure of the UTexture2D</param>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="generateMips">Whether to generate mipmaps</param>
    /// <returns>True if the operation was successful, false otherwise</returns>
    bool UpdateTexture_Internal(FTexturePlatformData* platformData,
                                UGPUContextObject* contextObject,
								bool generateMips);
private:
    std::shared_ptr<Gpu::IImage> mpImage;
};