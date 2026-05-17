#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Engine/DataAsset.h"

#include "GPUProgramAsset.generated.h"

UCLASS(BlueprintType)
class GPUWORKS_API UGPUProgramAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CLWorks")
	FString ProgramName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CLWorks")
	FString SourceCode;
};