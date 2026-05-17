#pragma once

#if WITH_EDITOR

#include "AssetTypeActions_Base.h"

#include "Assets/GPUProgramAsset.h"

class GPUWORKSEDITOR_API FGPUProgramAssetActions : public FAssetTypeActions_Base
{
public:
	FGPUProgramAssetActions(EAssetTypeCategories::Type InCategory)
		: mCategory(InCategory)
	{
	}
public:
	virtual FText GetName() const override 
	{
		return NSLOCTEXT("GPUWorks", "GPUProgramAssetActions", "GPU Program"); 
	}

	virtual FColor GetTypeColor() const override
	{
		return FColor::Orange;
	}

	virtual UClass* GetSupportedClass() const override
	{
		return UGPUProgramAsset::StaticClass();
	}

	virtual uint32 GetCategories() override
	{
		return mCategory;
	}

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, 
								 TSharedPtr<IToolkitHost> ToolkitHost) override;
private:
	EAssetTypeCategories::Type mCategory;
};

#endif