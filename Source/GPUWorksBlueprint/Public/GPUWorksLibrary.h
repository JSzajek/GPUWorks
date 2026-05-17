#pragma once

#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

#include "Interops/UE/GPUObjectDefines.h"

#include "GPUWorksLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGPUWorksBlueprint, Log, All);

class FGPUWorksBlueprintModule final : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
};


// Forward Declaration --------------------------
class UGPUContextObject;
class UGPUProgramObject;
class UGPUBufferObject;
class UGPUImageObject;

class UTexture2D;
class UTexture2DArray;
class UTextureRenderTarget2D;
// ----------------------------------------------

UCLASS(MinimalAPI)
class  UGPUWorksLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UGPUWorksLibrary(const class FObjectInitializer& ObjectInitializer);
	virtual void BeginDestroy() override;
public:
	void InitializeLibray();
	void DeinitializeLibray();
public:
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Custom Context")
	static UGPUContextObject* CreateCustomContext(int32 deviceIndex);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Program")
	static UGPUProgramObject* CreateProgram(UGPUProgramAsset* program, 
											const FString& kernelName,
											UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Integer Buffer")
	static UGPUBufferObject* CreateIntBuffer(const TArray<int32>& values,
											 UGPUAccessType access = UGPUAccessType::READ_WRITE,
											 UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
											 UGPUContextObject* contextOverride = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Float Buffer")
	static UGPUBufferObject* CreateFloatBuffer(const TArray<float>& values,
											  UGPUAccessType access = UGPUAccessType::READ_WRITE,
											  UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
											  UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Integer Vec2 Buffer")
	static UGPUBufferObject* CreateIntVector2Buffer(const TArray<FIntPoint>& values,
												   UGPUAccessType access = UGPUAccessType::READ_WRITE,
												   UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												   UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Integer Vec4 Buffer")
	static UGPUBufferObject* CreateIntVector4Buffer(const TArray<FIntVector4>& values,
												   UGPUAccessType access = UGPUAccessType::READ_WRITE,
												   UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												   UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Float Vec2 Buffer")
	static UGPUBufferObject* CreateVector2fBuffer(const TArray<FVector2f>& values,
												 UGPUAccessType access = UGPUAccessType::READ_WRITE,
												 UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												 UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Float Vec4 Buffer")
	static UGPUBufferObject* CreateVector4fBuffer(const TArray<FVector4f>& values,
												 UGPUAccessType access = UGPUAccessType::READ_WRITE,
												 UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												 UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Image")
	static UGPUImageObject* CreateImage(int32 width, 
										int32 height,
										int32 layers = 1,
										UGPUImageType type = UGPUImageType::Texture2D,
										UGPUImageFormat format = UGPUImageFormat::RGBA8,
										UGPUAccessType access = UGPUAccessType::READ_WRITE,
										UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Run Program")
	static bool RunProgram(UGPUProgramObject* program,
						   int64 dimensions,
						   const TArray<int64>& workCount);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Integer Buffer")
	static TArray<int32> ReadIntBuffer(UGPUBufferObject* buffer,
									   int32 numElements,
									   int32 offset = 0,
									   UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Float Buffer")
	static TArray<float> ReadFloatBuffer(UGPUBufferObject* buffer,
										 int32 numElements,
										 int32 offset = 0,
										 UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Integer Vec2 Buffer")
	static TArray<FIntPoint> ReadIntVector2Buffer(UGPUBufferObject* buffer,
												  int32 numElements,
												  int32 offset = 0,
												  UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Integer Vec4 Buffer")
	static TArray<FIntVector4> ReadIntVector4Buffer(UGPUBufferObject* buffer,
													int32 numElements,
													int32 offset = 0,
													UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Float Vec2 Buffer")
	static TArray<FVector2f> ReadVector2fBuffer(UGPUBufferObject* buffer,
												int32 numElements,
												int32 offset = 0,
												UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Float Vec4 Buffer")
	static TArray<FVector4f> ReadVector4fBuffer(UGPUBufferObject* buffer,
												int32 numElements,
												int32 offset = 0,
												UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Convert To Texture2D")
	static UTexture2D* ImageToTexture2D(UGPUImageObject* image,
										bool isSRGB = true,
									    bool generateMipMaps = false,
										UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Convert To Texture2DArray")
	static UTexture2DArray* ImageToTexture2DArray(UGPUImageObject* image,
												  bool isSRGB = true,
												  bool generateMipMaps = false,
												  UGPUContextObject* contextOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Write To RenderTarget2D")
	static bool WriteToRenderTarget2D(UTextureRenderTarget2D* output,
									  UGPUImageObject* image,
									  UGPUContextObject* contextOverride = nullptr);
private:
	static TObjectPtr<UGPUContextObject> mpGlobalGPUContext;
};