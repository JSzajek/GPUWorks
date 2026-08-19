#pragma once

#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

#include "Interops/UE/GPUObjectDefines.h"

#include "GPUWorksLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGPUWorksBlueprint, Log, All);

/// <summary>
/// GPU Works Blueprint Module.
/// </summary>
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

/// <summary>
/// GPU Works Blueprint Library.
/// </summary>
UCLASS(MinimalAPI)
class  UGPUWorksLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/// <summary>
	/// Constructor initializes UGPUWorksLibrary.
	/// </summary>
	/// <param name="ObjectInitializer">The object initializer</param>
	UGPUWorksLibrary(const class FObjectInitializer& ObjectInitializer);
	
	/// <summary>
	/// Initiates the object's destruction and performs any required cleanup.
	/// </summary>
	virtual void BeginDestroy() override;
public:
	/// <summary>
	/// Initializes the GPU Works library, must be called before any other function in this library is used.
	/// This is automatically called when using any function in this library, but can be called manually to ensure the library is initialized at a specific time.
	/// </summary>
	void InitializeLibray();

	/// <summary>
	/// Deinitilizes the GPU Works library, releasing any resources that were allocated.
	/// This is automatically called when the library is destroyed, but can be called manually to release resources at a specific time.
	/// </summary>
	void DeinitializeLibray();
public:
	/// <summary>
	/// Creates a custom context with the specified device index.
	/// If the context creation fails, this function will return null and log a warning.
	/// </summary>
	/// <param name="deviceIndex">The index of the device to create the context for</param>
	/// <returns>The created context object, or null if the creation failed</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Custom Context")
	static UGPUContextObject* CreateCustomContext(int32 deviceIndex);

	/// <summary>
	/// Creates a GPU program from the specified program asset and kernel name.
	/// </summary>
	/// <param name="program">The program asset</param>
	/// <param name="kernelName">The kernel name</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created program object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Program")
	static UGPUProgramObject* CreateProgram(UGPUProgramAsset* program, 
											const FString& kernelName,
											UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates and initializes a GPU buffer containing the provided integer values.
	/// </summary>
	/// <param name="values">Array of values</param>
	/// <param name="access">Access type</param>
	/// <param name="strategy">Memory allocation/usage strategy</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created buffer object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Integer Buffer")
	static UGPUBufferObject* CreateIntBuffer(const TArray<int32>& values,
											 UGPUAccessType access = UGPUAccessType::READ_WRITE,
											 UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
											 UGPUContextObject* contextOverride = nullptr);
	
	/// <summary>
	/// Creates and initializes a GPU buffer containing the provided float values.
	/// </summary>
	/// <param name="values">Array of values</param>
	/// <param name="access">Access type</param>
	/// <param name="strategy">Memory allocation/usage strategy</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created buffer object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Float Buffer")
	static UGPUBufferObject* CreateFloatBuffer(const TArray<float>& values,
											  UGPUAccessType access = UGPUAccessType::READ_WRITE,
											  UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
											  UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates and initializes a GPU buffer containing the provided int vec2 values.
	/// </summary>
	/// <param name="values">Array of values</param>
	/// <param name="access">Access type</param>
	/// <param name="strategy">Memory allocation/usage strategy</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created buffer object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Integer Vec2 Buffer")
	static UGPUBufferObject* CreateIntVector2Buffer(const TArray<FIntPoint>& values,
												   UGPUAccessType access = UGPUAccessType::READ_WRITE,
												   UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												   UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates and initializes a GPU buffer containing the provided int vec4 values.
	/// </summary>
	/// <param name="values">Array of values</param>
	/// <param name="access">Access type</param>
	/// <param name="strategy">Memory allocation/usage strategy</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created buffer object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Integer Vec4 Buffer")
	static UGPUBufferObject* CreateIntVector4Buffer(const TArray<FIntVector4>& values,
												   UGPUAccessType access = UGPUAccessType::READ_WRITE,
												   UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												   UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates and initializes a GPU buffer containing the provided float vec2 values.
	/// </summary>
	/// <param name="values">Array of values</param>
	/// <param name="access">Access type</param>
	/// <param name="strategy">Memory allocation/usage strategy</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created buffer object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Float Vec2 Buffer")
	static UGPUBufferObject* CreateVector2fBuffer(const TArray<FVector2f>& values,
												 UGPUAccessType access = UGPUAccessType::READ_WRITE,
												 UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												 UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates and initializes a GPU buffer containing the provided int vec4 values.
	/// </summary>
	/// <param name="values">Array of values</param>
	/// <param name="access">Access type</param>
	/// <param name="strategy">Memory allocation/usage strategy</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created buffer object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Float Vec4 Buffer")
	static UGPUBufferObject* CreateVector4fBuffer(const TArray<FVector4f>& values,
												 UGPUAccessType access = UGPUAccessType::READ_WRITE,
												 UGPUMemoryStrategy strategy = UGPUMemoryStrategy::STREAM,
												 UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates and initializes a GPU image with the specified dimensions, format, and type.
	/// </summary>
	/// <param name="width">The width of the image</param>
	/// <param name="height">The height of the image</param>
	/// <param name="layers">The number of layers in the image</param>
	/// <param name="type">The type of the image</param>
	/// <param name="format">The format of the image</param>
	/// <param name="access">The access type of the image</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created image object, or null if the creation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Create Image")
	static UGPUImageObject* CreateImage(int32 width, 
										int32 height,
										int32 layers = 1,
										UGPUImageType type = UGPUImageType::Texture2D,
										UGPUImageFormat format = UGPUImageFormat::RGBA8,
										UGPUAccessType access = UGPUAccessType::READ_WRITE,
										UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Runs the program with the specified dimensions and work count.
	/// </summary>
	/// <param name="program">The program to run</param>
	/// <param name="dimensions">The dimensions of the work</param>
	/// <param name="workCount">The number of work items</param>
	/// <returns>True if the program runs successfully, false otherwise</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Run Program")
	static bool RunProgram(UGPUProgramObject* program,
						   int64 dimensions,
						   const TArray<int64>& workCount);

	/// <summary>
	/// Reads the specified number of elements from the buffer into an array, starting at the specified offset.
	/// </summary>
	/// <param name="buffer">The buffer to read from</param>
	/// <param name="numElements">The number of elements to read</param>
	/// <param name="offset">The offset from which to start reading</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>An array containing the read elements</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Integer Buffer")
	static TArray<int32> ReadIntBuffer(UGPUBufferObject* buffer,
									   int32 numElements,
									   int32 offset = 0,
									   UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Reads the specified number of elements from the buffer into an array, starting at the specified offset.
	/// </summary>
	/// <param name="buffer">The buffer to read from</param>
	/// <param name="numElements">The number of elements to read</param>
	/// <param name="offset">The offset from which to start reading</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>An array containing the read elements</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Float Buffer")
	static TArray<float> ReadFloatBuffer(UGPUBufferObject* buffer,
										 int32 numElements,
										 int32 offset = 0,
										 UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Reads the specified number of elements from the buffer into an array, starting at the specified offset.
	/// </summary>
	/// <param name="buffer">The buffer to read from</param>
	/// <param name="numElements">The number of elements to read</param>
	/// <param name="offset">The offset from which to start reading</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>An array containing the read elements</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Integer Vec2 Buffer")
	static TArray<FIntPoint> ReadIntVector2Buffer(UGPUBufferObject* buffer,
												  int32 numElements,
												  int32 offset = 0,
												  UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Reads the specified number of elements from the buffer into an array, starting at the specified offset.
	/// </summary>
	/// <param name="buffer">The buffer to read from</param>
	/// <param name="numElements">The number of elements to read</param>
	/// <param name="offset">The offset from which to start reading</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>An array containing the read elements</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Integer Vec4 Buffer")
	static TArray<FIntVector4> ReadIntVector4Buffer(UGPUBufferObject* buffer,
													int32 numElements,
													int32 offset = 0,
													UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Reads the specified number of elements from the buffer into an array, starting at the specified offset.
	/// </summary>
	/// <param name="buffer">The buffer to read from</param>
	/// <param name="numElements">The number of elements to read</param>
	/// <param name="offset">The offset from which to start reading</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>An array containing the read elements</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Float Vec2 Buffer")
	static TArray<FVector2f> ReadVector2fBuffer(UGPUBufferObject* buffer,
												int32 numElements,
												int32 offset = 0,
												UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Reads the specified number of elements from the buffer into an array, starting at the specified offset.
	/// </summary>
	/// <param name="buffer">The buffer to read from</param>
	/// <param name="numElements">The number of elements to read</param>
	/// <param name="offset">The offset from which to start reading</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>An array containing the read elements</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Read Float Vec4 Buffer")
	static TArray<FVector4f> ReadVector4fBuffer(UGPUBufferObject* buffer,
												int32 numElements,
												int32 offset = 0,
												UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates a Texture2D from the GPU image, with options for sRGB color space and mipmap generation.
	/// </summary>
	/// <param name="image">The GPU image</param>
	/// <param name="isSRGB">Whether to use sRGB color space</param>
	/// <param name="generateMipMaps">Whether to generate mipmaps</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created Texture2D, or null if the operation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Convert To Texture2D")
	static UTexture2D* ImageToTexture2D(UGPUImageObject* image,
										bool isSRGB = true,
									    bool generateMipMaps = false,
										UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Creates a Texture2DArray from the GPU image, with options for sRGB color space and mipmap generation.
	/// </summary>
	/// <param name="image">The GPU image</param>
	/// <param name="isSRGB">Whether to use sRGB color space</param>
	/// <param name="generateMipMaps">Whether to generate mipmaps</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>The created Texture2DArray, or null if the operation fails</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Convert To Texture2DArray")
	static UTexture2DArray* ImageToTexture2DArray(UGPUImageObject* image,
												  bool isSRGB = true,
												  bool generateMipMaps = false,
												  UGPUContextObject* contextOverride = nullptr);

	/// <summary>
	/// Writes the contents of the GPU image to a TextureRenderTarget2D,
	/// allowing it to be used as a render target in Unreal Engine.
	/// </summary>
	/// <param name="output">The output TextureRenderTarget2D</param>
	/// <param name="image">The GPU image</param>
	/// <param name="contextOverride">The context override</param>
	/// <returns>True if the operation was successful</returns>
	UFUNCTION(BlueprintCallable, Category = "OpenCL", DisplayName = "Write To RenderTarget2D")
	static bool WriteToRenderTarget2D(UTextureRenderTarget2D* output,
									  UGPUImageObject* image,
									  UGPUContextObject* contextOverride = nullptr);
private:
	static TObjectPtr<UGPUContextObject> mpGlobalGPUContext;
};