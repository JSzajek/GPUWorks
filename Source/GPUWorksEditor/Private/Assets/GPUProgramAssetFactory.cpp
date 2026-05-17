#include "GPUProgramAssetFactory.h"

#if WITH_EDITOR

#include "Assets/GPUProgramAsset.h"

UGPUProgramAssetFactory::UGPUProgramAssetFactory(const FObjectInitializer& ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGPUProgramAsset::StaticClass();
}

UObject* UGPUProgramAssetFactory::FactoryCreateNew(UClass* Class,
												  UObject* InParent, 
												  FName Name, 
												  EObjectFlags Flags, 
												  UObject* Context, 
												  FFeedbackContext* Warn)
{
	UGPUProgramAsset* NewAsset = NewObject<UGPUProgramAsset>(InParent, Class, Name, Flags);
	if (NewAsset)
	{
		NewAsset->ProgramName = Name.ToString();
		NewAsset->SourceCode = "__kernel void example()\n{\n}";
	}

	return NewAsset;
}

#endif