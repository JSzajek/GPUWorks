#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GPU/GPUBuffer.h"
#include "GPU/GPUContext.h"

#include "GPUBufferObject.generated.h"

class UGPUContextObject;

/// <summary>
/// GPU buffer wrapper for Unreal Engine. This class manages a GPU buffer resource.
/// </summary>
UCLASS(BlueprintType)
class GPUWORKS_API UGPUBufferObject : public UObject
{
    GENERATED_BODY()
public:
    /// <summary>
	/// Initializes the GPU buffer with the specified size and access mode.
    /// The buffer will be created in the context of the provided GPU context object.
    /// </summary>
    /// <param name="contextObject">The GPU context object in which the buffer will be created</param>
    /// <param name="sizeBytes">The size of the buffer in bytes</param>
    /// <param name="bReadOnly">Whether the buffer is read-only</param>
    /// <param name="bWriteOnly">Whether the buffer is write-only</param>
    /// <returns>True if the buffer was successfully initialized</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool Initialize(UGPUContextObject* contextObject,
                    int64 sizeBytes,
                    bool bReadOnly = false,
                    bool bWriteOnly = false);

    /// <summary>
	/// Retrieves whether the GPU buffer is valid.
    /// </summary>
    /// <returns>True if the buffer is valid</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool IsValidBuffer() const;
    
    /// <summary>
    /// Retrieves the size of the GPU buffer in bytes.
    /// </summary>
    /// <returns>The size of the buffer in bytes</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    int64 GetSizeBytes() const;

    /// <summary>
	/// Copies the contents of another GPU buffer into this buffer.
    /// Both buffers must be created in the same GPU context.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="otherBuffer">The GPU buffer to copy from</param>
    UFUNCTION(BlueprintCallable, Category="GPU")
    void CopyBuffer(UGPUContextObject* contextObject,
                    UGPUBufferObject* otherBuffer);

    /// <summary>
	/// Uploads raw byte data to the GPU buffer.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="bytes">The raw byte data to upload</param>
    /// <param name="sizeBytes">The size of the data in bytes</param>
    /// <param name="offset">The offset in the buffer where the data will be uploaded</param>
    /// <returns>True if the upload was successful</returns>
    bool UploadRaw(UGPUContextObject* contextObject,
                   const void* bytes,
                   int64 sizeBytes,
                   int64 offset = 0);

    /// <summary>
	/// Uploads the contents of a byte array to the GPU buffer.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="bytes">The byte data to upload</param>
    /// <param name="offset">The offset in the buffer where the data will be uploaded</param>
    /// <returns>True if the upload was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool UploadBytes(UGPUContextObject* contextObject,
                     const TArray<uint8>& bytes,
                     int64 offset = 0);

    /// <summary>
	/// Downloads the contents of the GPU buffer into a raw byte array.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="outBytes">Output buffer to store the downloaded data</param>
    /// <param name="bytesToRead">The number of bytes to read from the GPU buffer</param>
    /// <param name="offset">The offset in the buffer from where to start reading</param>
    /// <returns>True if the download was successful</returns>
    bool DownloadRaw(UGPUContextObject* contextObject,
                     void* outBytes,
                     int64 bytesToRead = -1,
                     int64 offset = 0);

    /// <summary>
    /// Downloads the contents of the GPU buffer into a byte array.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="outBytes">Output array to store the downloaded data</param>
    /// <param name="bytesToRead">The number of bytes to read from the GPU buffer</param>
    /// <param name="offset">The offset in the buffer from where to start reading</param>
    /// <returns>True if the download was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool DownloadBytes(UGPUContextObject* contextObject,
                       TArray<uint8>& outBytes,
                       int64 bytesToRead = -1,
                       int64 offset = 0);

    /// <summary>
    /// Uploads the contents of a float array to the GPU buffer.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="values">The float data to upload</param>
    /// <param name="offsetBytes">The offset in the buffer where the data will be uploaded</param>
    /// <returns>True if the upload was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool UploadFloatArray(UGPUContextObject* contextObject,
                          const TArray<float>& values,
                          int64 offsetBytes = 0);

    /// <summary>
    /// Downloads the contents of the GPU buffer into a float array.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="outValues">Output array to store the downloaded data</param>
    /// <param name="floatCount">The number of floats to read from the GPU buffer</param>
    /// <param name="offsetBytes">The offset in the buffer from where to start reading</param>
    /// <returns>True if the download was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool DownloadFloatArray(UGPUContextObject* contextObject,
                            TArray<float>& outValues,
                            int32 floatCount = -1,
                            int64 offsetBytes = 0);
public:
    /// <summary>
	/// Retrieves the underlying GPU buffer resource.
    /// </summary>
    /// <returns>The shared pointer to the GPU buffer</returns>
    inline std::shared_ptr<Gpu::IBuffer> GetBuffer() const { return mpBuffer; }
private:
    std::shared_ptr<Gpu::IBuffer> mpBuffer;
};