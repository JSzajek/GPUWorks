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

UCLASS(BlueprintType)
class GPUWORKS_API UGPUProgramObject : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool BuildFromSource(UGPUContextObject* contextObject,
                         const FString& source);

    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool BuildFromAsset(UGPUContextObject* contextObject,
						UGPUProgramAsset* asset);
	

	UFUNCTION(BlueprintCallable, Category = "GPU")
    void SetKernel(const FString& kernelName);

    UFUNCTION(BlueprintCallable, Category = "GPU")
    bool IsValidProgram() const;

    UFUNCTION(BlueprintCallable, Category = "GPU")
    FString GetLastBuildLog() const;

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
    std::shared_ptr<Gpu::IProgram> GetProgram() const { return Program; }
    std::shared_ptr<Gpu::IKernel> GetKernel() const { return mpKernel; }
private:
    std::shared_ptr<Gpu::IProgram> Program = nullptr;
    std::shared_ptr<Gpu::IKernel> mpKernel = nullptr;
    FString LastBuildLog;
};