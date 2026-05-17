#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"

#include "Factories/Factory.h"

#include "GPUProgramAssetFactory.generated.h"


UCLASS()
class GPUWORKSEDITOR_API UGPUProgramAssetFactory : public UFactory
{
	GENERATED_BODY()
public:
	UGPUProgramAssetFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, 
									  UObject* InParent, 
									  FName Name, 
									  EObjectFlags Flags, 
									  UObject* Context, 
									  FFeedbackContext* Warn) override;
};

#endif