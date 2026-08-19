#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GPU/GPUCore.h"
#include "GPU/GPUContext.h"
#include "GPU/GPUQueue.h"
#include "GPU/GPUFactory.h"

#include "GPUContextObject.generated.h"

/// <summary>
/// Enum representing the GPU backend.
/// </summary>
UENUM(BlueprintType)
enum class EGPUBackend : uint8
{
    Unknown,

    OpenCL,
    CUDA
};

/// <summary>
/// GPU context wrapper for Unreal Engine. This class manages a GPU context and its associated resources.
/// </summary>
UCLASS(BlueprintType)
class GPUWORKS_API UGPUContextObject : public UObject
{
    GENERATED_BODY()
public:
    /// <summary>
	/// Initializes the GPU context with the specified backend and device index.
    /// This will create a GPU context for the selected device.
    /// </summary>
    /// <param name="backend">The GPU backend to use</param>
    /// <param name="deviceIndex">The index of the GPU device to use</param>
    /// <returns>True if the context was successfully initialized</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool Initialize(EGPUBackend backend,
                    int32 deviceIndex = 0);

    /// <summary>
    /// Checks whether the GPU context is valid.
    /// </summary>
    /// <returns>True if the context is valid</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool IsValidContext() const;

    /// <summary>
	/// Retrieves the name of the GPU device associated with this context.
    /// </summary>
    /// <returns>The name of the GPU device</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    FString GetDeviceName() const;

    /// <summary>
	/// Creates a default GPU command queue for this contextTrue. 
    /// The queue will be created with default properties and can be used to submit GPU commands.
    /// </summary>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category="GPU")
    bool CreateDefaultQueue();

    /// <summary>
	/// Checks whether the GPU context supports image resources.
    /// </summary>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool HasImageSupport() const;

	/// <summary>
	/// Retrieves the GPU backend used by this context.
	/// </summary>
	/// <returns>The GPU backend</returns>
	EGPUBackend GetGPUBackend() const { return mBackend; }
public:
    /// <summary>
	/// Retrieves the underlying GPU core resource.
    /// </summary>
    /// <returns>The shared pointer to the GPU core</returns>
    inline std::shared_ptr<Gpu::ICore> GetCore() const { return mpCore; }

    /// <summary>
    /// Retrieves the underlying GPU device resource.
    /// </summary>
    /// <returns>The shared pointer to the GPU device</returns>
    inline std::shared_ptr<Gpu::IDevice> GetDevice() const { return mpDevice; }

    /// <summary>
    /// Retrieves the underlying GPU context resource.
    /// </summary>
    /// <returns>The shared pointer to the GPU context</returns>
    inline std::shared_ptr<Gpu::IContext> GetContext() const { return mpContext; }

    /// <summary>
    /// Retrieves the underlying GPU default queue resource.
    /// </summary>
    /// <returns>The shared pointer to the GPU default queue</returns>
    inline std::shared_ptr<Gpu::IQueue> GetDefaultQueue() const { return mpDefaultQueue; }
private:
	EGPUBackend mBackend = EGPUBackend::Unknown;

    std::shared_ptr<Gpu::ICore> mpCore;
    std::shared_ptr<Gpu::IDevice> mpDevice;
    std::shared_ptr<Gpu::IContext> mpContext;
    std::shared_ptr<Gpu::IQueue> mpDefaultQueue;
};