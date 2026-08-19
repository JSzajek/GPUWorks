// UEInterop/UGpuProgramObject.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GPU/GPUProgram.h"
#include "GPU/GPUKernel.h"
#include "GPU/GPUContext.h"

#include "GPUProgramObject.generated.h"

class UGPUContextObject;
class UGPUBufferObject;
class UGPUImageObject;
class UGPUProgramAsset;

/// <summary>
/// GPU program wrapper for Unreal Engine. This class manages a GPU program and its associated kernels and resources.
/// </summary>
UCLASS(BlueprintType)
class GPUWORKS_API UGPUProgramObject : public UObject
{
    GENERATED_BODY()
public:
    /// <summary>
	/// Build the GPU program from the provided source code.
	/// 
	/// NOTE: The source code should be in the language corresponding to the GPU context's backend.
    /// </summary>
    /// <param name="contextObject">The GPU context object</param>
    /// <param name="source">The source code</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool BuildFromSource(UGPUContextObject* contextObject,
                         const FString& source);

    /// <summary>
	/// Build the GPU program from the provided program asset.
    /// </summary>
	/// <param name="contextObject">The GPU context object</param>
    /// <param name="asset">The program asset</param>
    /// <returns>True if the operation was successful</returns>
    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool BuildFromAsset(UGPUContextObject* contextObject,
						UGPUProgramAsset* asset);
	
	/// <summary>
	/// Sets the active kernel for this GPU program by name.
	/// </summary>
	/// <param name="kernelName">The kernel name</param>
	UFUNCTION(BlueprintCallable, Category = "GPU")
    void SetKernel(const FString& kernelName);

    /// <summary>
	/// Checks whether the GPU program is valid and has been successfully built.
    /// </summary>
    /// <returns>True if the GPU program is valid</returns>
    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool IsValidProgram() const;

    /// <summary>
    /// Retrieves the last build log for the GPU program.
    /// </summary>
    /// <returns>The build log</returns>
    UFUNCTION(BlueprintCallable, Category = "GPU")
    FString GetLastBuildLog() const;

    /// <summary>
	/// Checks whether the program has the specified kernel.
    /// </summary>
    /// <param name="kernelName">The kernel name</param>
    /// <returns>True if the program has the kernel</returns>
    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool HasKernel(const FString& kernelName) const;

    /// <summary>
	/// Sets a integer argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="integer">The integer</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Integer Argument")
	bool SetIntArg(int32 index,
				   int32 integer);

	/// <summary>
	/// Sets a float argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="scalar">The float</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Float Argument")
	bool SetFloatArg(int32 index,
					 float scalar);

	/// <summary>
	/// Sets a integer vec2 argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="vec">The vector</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Integer Vec2 Argument")
	bool SetIntVector2Arg(int32 index,
						  const FIntPoint& vec);

	/// <summary>
	/// Sets a integer vec4 argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="vec">The vector</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Integer Vec4 Argument")
	bool SetIntVector4Arg(int32 index,
						  const FIntVector4& vec);

	/// <summary>
	/// Sets a float vec2 argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="vec">The vector</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Float Vec2 Argument")
	bool SetVector2fArg(int32 index,
						const FVector2f& vec);

	/// <summary>
	/// Sets a float vec4 argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="vec">The vector</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Float Vec4 Argument")
	bool SetVector4fArg(int32 index,
						const FVector4f& vec);

	/// <summary>
	/// Sets a buffer argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="buffer">The buffer</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Buffer Argument")
	bool SetBufferArg(int32 index,
					  UGPUBufferObject* buffer);

	/// <summary>
	/// Sets a image argument.
	/// </summary>
	/// <param name="index">The argument index</param>
	/// <param name="image">The image</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "GPU", DisplayName = "Set Image Argument")
	bool SetImageArg(int32 index,
					 UGPUImageObject* image);
public:
	/// <summary>
	/// Retrieves the underlying GPU program resource.
	/// </summary>
	/// <returns>The shared pointer to the GPU program</returns>
    inline std::shared_ptr<Gpu::IProgram> GetProgram() const { return mpProgram; }

	/// <summary>
	/// Retrieves the underlying GPU kernel resource.
	/// </summary>
	/// <returns>The shared pointer to the GPU kernel</returns>
    inline std::shared_ptr<Gpu::IKernel> GetKernel() const { return mpKernel; }
private:
    std::shared_ptr<Gpu::IProgram> mpProgram = nullptr;
    std::shared_ptr<Gpu::IKernel> mpKernel = nullptr;
    FString mLastBuildLog;
};