#include "GPUWorksLibrary.h"

#include "GPUWorksLib.h"

#include <memory>

DEFINE_LOG_CATEGORY(LogGPUWorksBlueprint);

#define LOCTEXT_NAMESPACE "FGPUWorksBlueprintModule"

TObjectPtr<UGPUContextObject> UGPUWorksLibrary::mpGlobalGPUContext = nullptr;

EGpuPixelFormat ConvertToPixelFormat(UGPUImageFormat format)
{
	switch (format)
	{
		case UGPUImageFormat::R8:
			return EGpuPixelFormat::R8;
		case UGPUImageFormat::RGBA8:
			return EGpuPixelFormat::RGBA8;
		case UGPUImageFormat::R16:
			return EGpuPixelFormat::R16F;
		case UGPUImageFormat::RGBA16:
			return EGpuPixelFormat::RGBA16F;
		case UGPUImageFormat::R32:
			return EGpuPixelFormat::R32F;
		case UGPUImageFormat::RGBA32:
			return EGpuPixelFormat::RGBA32F;
		default:
			return EGpuPixelFormat::Unknown;
	}
}

void FGPUWorksBlueprintModule::StartupModule()
{
}

void FGPUWorksBlueprintModule::ShutdownModule()
{
}

UGPUWorksLibrary::UGPUWorksLibrary(const class FObjectInitializer& ObjectInitializer)
{
	InitializeLibray();
}

void UGPUWorksLibrary::BeginDestroy()
{
	Super::BeginDestroy();

	DeinitializeLibray();
}

void UGPUWorksLibrary::InitializeLibray()
{
	mpGlobalGPUContext = NewObject<UGPUContextObject>(GetTransientPackage(), NAME_None, RF_Transient);
	mpGlobalGPUContext->Initialize(EGpuBackend::OpenCL);
}

void UGPUWorksLibrary::DeinitializeLibray()
{
	if (mpGlobalGPUContext)
	{
		mpGlobalGPUContext->ConditionalBeginDestroy();
		mpGlobalGPUContext = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGPUWorksBlueprintModule, GPUWorksBlueprint)



UGPUContextObject* UGPUWorksLibrary::CreateCustomContext(int32 deviceIndex)
{
	UGPUContextObject* context = NewObject<UGPUContextObject>(GetTransientPackage(), NAME_None, RF_Transient);
	context->Initialize(EGpuBackend::OpenCL);

	if (!context->IsValidContext())
	{
		context->ConditionalBeginDestroy();
		return nullptr;
	}
	return context;
}

UGPUProgramObject* UGPUWorksLibrary::CreateProgram(UGPUProgramAsset* asset,
												   const FString& kernelName, 
												   UGPUContextObject* contextOverride)
{
	if (!asset)
	{
		UE_LOG(LogGPUWorksBlueprint, Warning, TEXT("Invalid Program Asset!"));
		return nullptr;
	}

	UGPUProgramObject* program = NewObject<UGPUProgramObject>(GetTransientPackage(), NAME_None, RF_Transient);
	program->BuildFromSource(contextOverride ? contextOverride : mpGlobalGPUContext.Get(),
							 asset->SourceCode);

	program->SetKernel(kernelName);

	if (!program->IsValidProgram())
	{
		program->ConditionalBeginDestroy();
		return nullptr;
	}
	return program;
}

UGPUBufferObject* UGPUWorksLibrary::CreateIntBuffer(const TArray<int32>& values,
												    UGPUAccessType access,
												    UGPUMemoryStrategy strategy,
												    UGPUContextObject* contextOverride)
{
	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();

	UGPUBufferObject* buffer = NewObject<UGPUBufferObject>(GetTransientPackage(), NAME_None, RF_Transient);
	buffer->Initialize(context,
					   values.NumBytes());

	buffer->UploadRaw(context, 
					  values.GetData(),
					  values.NumBytes());

	if (!buffer->GetBuffer())
	{
		buffer->ConditionalBeginDestroy();
		return nullptr;
	}
	return buffer;
}

UGPUBufferObject* UGPUWorksLibrary::CreateFloatBuffer(const TArray<float>& values,
													  UGPUAccessType access,
													  UGPUMemoryStrategy strategy,
													  UGPUContextObject* contextOverride)
{
	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();

	UGPUBufferObject* buffer = NewObject<UGPUBufferObject>(GetTransientPackage(), NAME_None, RF_Transient);
	buffer->Initialize(context,
					   values.NumBytes());

	buffer->UploadFloatArray(context,
							 values);

	if (!buffer->GetBuffer())
	{
		buffer->ConditionalBeginDestroy();
		return nullptr;
	}
	return buffer;
}

UGPUBufferObject* UGPUWorksLibrary::CreateIntVector2Buffer(const TArray<FIntPoint>& values,
	UGPUAccessType access,
	UGPUMemoryStrategy strategy,
	UGPUContextObject* contextOverride)
{
	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();

	UGPUBufferObject* buffer = NewObject<UGPUBufferObject>(GetTransientPackage(), NAME_None, RF_Transient);
	buffer->Initialize(context,
					   values.NumBytes());

	buffer->UploadRaw(context, 
					  values.GetData(),
					  values.NumBytes());

	if (!buffer->GetBuffer())
	{
		buffer->ConditionalBeginDestroy();
		return nullptr;
	}
	return buffer;
}

UGPUBufferObject* UGPUWorksLibrary::CreateIntVector4Buffer(const TArray<FIntVector4>& values,
														   UGPUAccessType access, 
														   UGPUMemoryStrategy strategy,
														   UGPUContextObject* contextOverride)
{
	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();

	UGPUBufferObject* buffer = NewObject<UGPUBufferObject>(GetTransientPackage(), NAME_None, RF_Transient);
	buffer->Initialize(context,
					   values.NumBytes());

	buffer->UploadRaw(context, 
					  values.GetData(),
					  values.NumBytes());

	if (!buffer->GetBuffer())
	{
		buffer->ConditionalBeginDestroy();
		return nullptr;
	}
	return buffer;
}

UGPUBufferObject* UGPUWorksLibrary::CreateVector2fBuffer(const TArray<FVector2f>& values,
													     UGPUAccessType access, 
													     UGPUMemoryStrategy strategy,
													     UGPUContextObject* contextOverride)
{
	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();

	UGPUBufferObject* buffer = NewObject<UGPUBufferObject>(GetTransientPackage(), NAME_None, RF_Transient);
	buffer->Initialize(context,
					   values.NumBytes());

	buffer->UploadRaw(context, 
					  values.GetData(),
					  values.NumBytes());

	if (!buffer->GetBuffer())
	{
		buffer->ConditionalBeginDestroy();
		return nullptr;
	}
	return buffer;
}

UGPUBufferObject* UGPUWorksLibrary::CreateVector4fBuffer(const TArray<FVector4f>& values,
													     UGPUAccessType access, 
													     UGPUMemoryStrategy strategy,
													     UGPUContextObject* contextOverride)
{
	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();

	UGPUBufferObject* buffer = NewObject<UGPUBufferObject>(GetTransientPackage(), NAME_None, RF_Transient);
	buffer->Initialize(context,
					   values.NumBytes());

	buffer->UploadRaw(context, 
					  values.GetData(),
					  values.NumBytes());

	if (!buffer->GetBuffer())
	{
		buffer->ConditionalBeginDestroy();
		return nullptr;
	}
	return buffer;
}

UGPUImageObject* UGPUWorksLibrary::CreateImage(int32 width,
											   int32 height,
											   int32 layers,
											   UGPUImageType type, 
											   UGPUImageFormat format, 
											   UGPUAccessType access, 
											   UGPUContextObject* contextOverride)
{
	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();

	if (!context->HasImageSupport())
	{
		UE_LOG(LogGPUWorksBlueprint, Warning, TEXT("Failed Image Object Creation: No Image Support!"));
	}
	
	UGPUImageObject* image = NewObject<UGPUImageObject>(GetTransientPackage(), NAME_None, RF_Transient);
	EGpuPixelFormat pixelFormat = ConvertToPixelFormat(format);

	switch (type)
	{
		case UGPUImageType::Texture2D:
		{
			image->CreateImage2D(context,
								 width,
								 height,
								 pixelFormat);
			break;
		}
		case UGPUImageType::Texture2DArray:
		{
			image->CreateImage2DArray(context,
									  width,
									  height,
									  layers,
									  pixelFormat);
			break;
		}
		case UGPUImageType::Texture3D:
		{
			image->CreateImage3D(context,
								 width,
								 height,
								 layers,
								 pixelFormat);
			break;
		}
	}

	if (!image->GetImage())
	{
		image->ConditionalBeginDestroy();
		return nullptr;
	}
	return image;
}

bool UGPUWorksLibrary::RunProgram(UGPUProgramObject* program, 
								  int64 dimensions, 
								  const TArray<int64>& workCount)
{
	if (!program->IsValidProgram())
	{
		return false;
	}

	if (workCount.Num() != dimensions)
	{
		UE_LOG(LogGPUWorksBlueprint, Warning, TEXT("Mismatched Dimensions and Work Size: %d vs %d"), dimensions, workCount.Num());
		return false;
	}

	std::shared_ptr<Gpu::IQueue> queue = mpGlobalGPUContext->GetDefaultQueue();

	Gpu::DispatchDescription dispatchDesc;
	dispatchDesc.Dim = dimensions;

	uint32_t cnt = FMath::Min((uint32_t)dimensions, 3u);
	for (uint32_t i = 0; i < cnt; ++i)
	{
		dispatchDesc.Global[i] = workCount[i];
	}
	
	std::shared_ptr<Gpu::IEvent> event = queue->Dispatch(*program->GetKernel(), dispatchDesc);
	event->Wait();

	return true;
}

TArray<int32> UGPUWorksLibrary::ReadIntBuffer(UGPUBufferObject* buffer,
	int32 numElements,
	int32 offset,
	UGPUContextObject* contextOverride)
{
	TArray<int32> output;
	output.SetNumZeroed(numElements);

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	buffer->DownloadRaw(context, output.GetData(), numElements * sizeof(int32), offset);
	return output;
}

TArray<float> UGPUWorksLibrary::ReadFloatBuffer(UGPUBufferObject* buffer,
											    int32 numElements,
											    int32 offset,
											    UGPUContextObject* contextOverride)
{

	TArray<float> output;
	output.SetNumZeroed(numElements);

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	buffer->DownloadRaw(context, output.GetData(), numElements * sizeof(float), offset);
	return output;
}

TArray<FIntPoint> UGPUWorksLibrary::ReadIntVector2Buffer(UGPUBufferObject* buffer, 
														 int32 numElements, 
														 int32 offset,
														 UGPUContextObject* contextOverride)
{
	TArray<FIntPoint> output;
	output.SetNumZeroed(numElements);

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	buffer->DownloadRaw(context, output.GetData(), numElements * sizeof(FIntPoint), offset);
	return output;
}

TArray<FIntVector4> UGPUWorksLibrary::ReadIntVector4Buffer(UGPUBufferObject* buffer, 
														   int32 numElements, 
														   int32 offset,
														   UGPUContextObject* contextOverride)
{

	TArray<FIntVector4> output;
	output.SetNumZeroed(numElements);

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	buffer->DownloadRaw(context, output.GetData(), numElements * sizeof(FIntVector4), offset);
	return output;
}

TArray<FVector2f> UGPUWorksLibrary::ReadVector2fBuffer(UGPUBufferObject* buffer, 
													   int32 numElements, 
													   int32 offset,
													   UGPUContextObject* contextOverride)
{
	TArray<FVector2f> output;
	output.SetNumZeroed(numElements);

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	buffer->DownloadRaw(context, output.GetData(), numElements * sizeof(FVector2f), offset);
	return output;
}

TArray<FVector4f> UGPUWorksLibrary::ReadVector4fBuffer(UGPUBufferObject* buffer, 
													   int32 numElements, 
													   int32 offset,
													   UGPUContextObject* contextOverride)
{
	TArray<FVector4f> output;
	output.SetNumZeroed(numElements);

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	buffer->DownloadRaw(context, output.GetData(), numElements * sizeof(FVector4f), offset);
	return output;
}

UTexture2D* UGPUWorksLibrary::ImageToTexture2D(UGPUImageObject* image, 
											   bool isSRGB,
											   bool generateMipMaps,
											   UGPUContextObject* contextOverride)
{
	if (image->GetImage() == nullptr)
	{
		UE_LOG(LogGPUWorksBlueprint, Warning, TEXT("Invalid Image!"));
		return nullptr;
	}

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	return image->CreateTexture2D(context, isSRGB, generateMipMaps);
}

UTexture2DArray* UGPUWorksLibrary::ImageToTexture2DArray(UGPUImageObject* image,
														 bool isSRGB,
														 bool generateMipMaps,
														 UGPUContextObject* contextOverride)
{
	if (image->GetImage() == nullptr)
	{
		UE_LOG(LogGPUWorksBlueprint, Warning, TEXT("Invalid Image!"));
		return nullptr;
	}

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	return image->CreateTexture2DArray(context, isSRGB, generateMipMaps);
}

bool UGPUWorksLibrary::WriteToRenderTarget2D(UTextureRenderTarget2D* output,
											 UGPUImageObject* image,
											 UGPUContextObject* contextOverride)
{
	if (image->GetImage() == nullptr)
	{
		UE_LOG(LogGPUWorksBlueprint, Warning, TEXT("Invalid Image!"));
		return false;
	}

	UGPUContextObject* context = contextOverride ? contextOverride : mpGlobalGPUContext.Get();
	return image->WriteToRenderTarget2D(context, output);
}