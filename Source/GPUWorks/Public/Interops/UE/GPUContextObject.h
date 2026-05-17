#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GPU/GPUCore.h"
#include "GPU/GPUContext.h"
#include "GPU/GPUQueue.h"
#include "GPU/GPUFactory.h"

#include "GPUContextObject.generated.h"

UENUM(BlueprintType)
enum class EGpuBackend : uint8
{
    OpenCL,
    CUDA
};

UCLASS(BlueprintType)
class GPUWORKS_API UGPUContextObject : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool Initialize(EGpuBackend backend,
                    int32 deviceIndex = 0);

    UFUNCTION(BlueprintCallable, Category="GPU")
    bool IsValidContext() const;

    UFUNCTION(BlueprintCallable, Category="GPU")
    FString GetDeviceName() const;

    UFUNCTION(BlueprintCallable, Category="GPU")
    bool CreateDefaultQueue();

    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool HasImageSupport() const;

    std::shared_ptr<Gpu::ICore> GetCore() const { return mpCore; }
    std::shared_ptr<Gpu::IDevice> GetDevice() const { return mpDevice; }
    std::shared_ptr<Gpu::IContext> GetContext() const { return mpContext; }
    std::shared_ptr<Gpu::IQueue> GetDefaultQueue() const { return mpDefaultQueue; }
private:
    std::shared_ptr<Gpu::ICore> mpCore;
    std::shared_ptr<Gpu::IDevice> mpDevice;
    std::shared_ptr<Gpu::IContext> mpContext;
    std::shared_ptr<Gpu::IQueue> mpDefaultQueue;
};