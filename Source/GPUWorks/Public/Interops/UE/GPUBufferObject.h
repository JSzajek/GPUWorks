#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GPU/GPUBuffer.h"
#include "GPU/GPUContext.h"

#include "GPUBufferObject.generated.h"

class UGPUContextObject;

UCLASS(BlueprintType)
class GPUWORKS_API UGPUBufferObject : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool Initialize(UGPUContextObject* contextObject,
                    int64 sizeBytes,
                    bool bReadOnly = false,
                    bool bWriteOnly = false);

    UFUNCTION(BlueprintCallable, Category="GPU")
    bool IsValidBuffer() const;

    UFUNCTION(BlueprintCallable, Category="GPU")
    int64 GetSizeBytes() const;

    bool UploadRaw(UGPUContextObject* contextObject,
                   const void* bytes,
                   int64 sizeBytes,
                   int64 offset = 0);

    UFUNCTION(BlueprintCallable, Category="GPU")
    bool UploadBytes(UGPUContextObject* contextObject,
                     const TArray<uint8>& bytes,
                     int64 offset = 0);

    bool DownloadRaw(UGPUContextObject* contextObject,
                     void* outBytes,
                     int64 bytesToRead = -1,
                     int64 offset = 0);

    UFUNCTION(BlueprintCallable, Category="GPU")
    bool DownloadBytes(UGPUContextObject* contextObject,
                       TArray<uint8>& outBytes,
                       int64 bytesToRead = -1,
                       int64 offset = 0);

    UFUNCTION(BlueprintCallable, Category="GPU")
    bool UploadFloatArray(UGPUContextObject* contextObject,
                          const TArray<float>& values,
                          int64 offsetBytes = 0);

    UFUNCTION(BlueprintCallable, Category="GPU")
    bool DownloadFloatArray(UGPUContextObject* contextObject,
                            TArray<float>& outValues,
                            int32 floatCount = -1,
                            int64 offsetBytes = 0);
public:
    std::shared_ptr<Gpu::IBuffer> GetBuffer() const { return Buffer; }
private:
    std::shared_ptr<Gpu::IBuffer> Buffer;
};