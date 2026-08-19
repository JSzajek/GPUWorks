#pragma once

#if WITH_EDITOR

#include "AssetTypeActions_Base.h"

#include "Assets/GPUProgramAsset.h"

/// <summary>
/// GPU Program Asset Actions, used to register the asset type actions for GPU Program assets in the Unreal Editor.
/// </summary>
class GPUWORKSEDITOR_API FGPUProgramAssetActions : public FAssetTypeActions_Base
{
public:
	/// <summary>
	/// Constructor initializing a FGPUProgramAssetActions with the given asset category.
	/// </summary>
	/// <param name="InCategory">The asset category</param>
	FGPUProgramAssetActions(EAssetTypeCategories::Type InCategory)
		: mCategory(InCategory)
	{
	}
public:
	/// <summary>
	/// Retrieves the name of the asset type.
	/// </summary>
	/// <returns>The name</returns>
	virtual FText GetName() const override 
	{
		return NSLOCTEXT("GPUWorks", "GPUProgramAssetActions", "GPU Program"); 
	}

	/// <summary>
	/// Retrieves the type color of the asset.
	/// </summary>
	/// <returns>The type coloy</returns>
	virtual FColor GetTypeColor() const override
	{
		return FColor::Orange;
	}

	/// <summary>
	/// Retrieves the supported class of the asset type.
	/// </summary>
	/// <returns>Tje supported class</returns>
	virtual UClass* GetSupportedClass() const override
	{
		return UGPUProgramAsset::StaticClass();
	}

	/// <summary>
	/// Retrieves the categories of the asset type.
	/// </summary>
	/// <returns>The categories</returns>
	virtual uint32 GetCategories() override
	{
		return mCategory;
	}

	/// <summary>
	/// Opens the asset editor for the specified objects.
	/// If EditWithinLevelEditor is valid, the world-centric editor will be used.
	/// </summary>
	/// <param name="InObjects">The input objects</param>
	/// <param name="ToolkitHost">The toolkit host</param>
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, 
								 TSharedPtr<IToolkitHost> ToolkitHost) override;
private:
	EAssetTypeCategories::Type mCategory;
};

#endif