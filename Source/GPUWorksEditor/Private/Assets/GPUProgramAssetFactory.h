#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"

#include "Factories/Factory.h"

#include "GPUProgramAssetFactory.generated.h"

/// <summary>
/// GPU Program Asset Factory is responsible for creating new GPU Program assets in the Unreal Editor.
/// </summary>
UCLASS()
class GPUWORKSEDITOR_API UGPUProgramAssetFactory : public UFactory
{
	GENERATED_BODY()
public:
	/// <summary>
	/// Constructor initializing a UGPUProgramAssetFactory.
	/// </summary>
	/// <param name="ObjectInitializer">The object initializer</param>
	UGPUProgramAssetFactory(const FObjectInitializer& ObjectInitializer);

	/// <summary>
	/// Create a new object by class.
	/// </summary>
	/// <param name="Class">The class of the object</param>
	/// <param name="InParent">The parent object</param>
	/// <param name="Name">The name of the object</param>
	/// <param name="Flags">The object flags</param>
	/// <param name="Context">The context object</param>
	/// <param name="Warn">The feedback context</param>
	/// <returns>The created object</returns>
	virtual UObject* FactoryCreateNew(UClass* Class, 
									  UObject* InParent, 
									  FName Name, 
									  EObjectFlags Flags, 
									  UObject* Context, 
									  FFeedbackContext* Warn) override;
};

#endif