#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Engine/DataAsset.h"

#include "Interops/UE/GPUContextObject.h"

#include "GPUProgramAsset.generated.h"

/// <summary>
/// GPU Program Language is an enumeration that defines the different programming languages that can be used to write GPU programs.
/// </summary>
UENUM(BlueprintType)
enum class EGPUProgramLanguage : uint8
{
    OpenCL_C,

	// Not Supported Yet
    CUDA_C,
    SharedGPUDSL,
};

/// <summary>
/// GPU Program Asset is a data asset that contains the source code for a GPU program, 
/// as well as the language the source code is written in and any build options to be used when compiling the program.
/// This asset can be used to store GPU programs in a way that allows them to be easily edited and reused across multiple contexts and backends.
/// </summary>
UCLASS(BlueprintType)
class GPUWORKS_API UGPUProgramAsset : public UDataAsset
{
	GENERATED_BODY()
public:
    /// <summary>
	/// Retrieves the source code for the specified backend if it matches the 
    /// program asset's language, otherwise returns an empty string and logs a warning.
    /// </summary>
    /// <param name="backend">The backend for which to retrieve the source code</param>
    /// <returns>The source code for the specified backend, or an empty string if the backend does not match the program asset's language</returns>
    FString GetSourceCodeForBackend(EGPUBackend backend) const;

    /// <summary>
    /// Retrieves the source code for the specified backend if it matches the
    /// program asset's language, otherwise returns an empty string and logs a warning.
    /// </summary>
    /// <param name="backend">The backend for which to retrieve the source code</param>
    /// <returns>The source code for the specified backend, or an empty string if the backend does not match the program asset's language</returns>
    FString GetSourceCodeForBackend(Gpu::Backend backend) const;

    /// <summary>
	/// Sets the source code for the specified backend if it matches the
    /// program asset's language, otherwise logs a warning and does not set the source code.
    /// </summary>
    /// <param name="backend">The backend for which to set the source code</param>
    /// <param name="source">The source code to set for the specified backend</param>
    void SetSourceCodeForBackend(EGPUBackend backend,
                                 const FString& source);
public:
    UPROPERTY(EditAnywhere)
    EGPUProgramLanguage Language = EGPUProgramLanguage::OpenCL_C;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GPUWorks")
	FString ProgramName;

    UPROPERTY(EditAnywhere, meta=(MultiLine=true), Category = "GPUWorks")
    FString OpenCLSource;

    UPROPERTY(EditAnywhere, meta=(MultiLine=true), Category = "GPUWorks")
    FString CUDASource;

    UPROPERTY(EditAnywhere, Category = "GPUWorks")
    TArray<FString> BuildOptions;
};