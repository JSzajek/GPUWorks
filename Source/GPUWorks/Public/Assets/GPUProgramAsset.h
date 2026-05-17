#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Engine/DataAsset.h"

#include "Interops/UE/GPUContextObject.h"

#include "GPUProgramAsset.generated.h"

UENUM(BlueprintType)
enum class EGPUProgramLanguage : uint8
{
    OpenCL_C,

	// Not Supported Yet
    CUDA_C,
    SharedGPUDSL,
};

UCLASS(BlueprintType)
class GPUWORKS_API UGPUProgramAsset : public UDataAsset
{
	GENERATED_BODY()
public:
    FString GetSourceCodeForBackend(EGPUBackend backend) const;
    FString GetSourceCodeForBackend(Gpu::Backend backend) const;

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