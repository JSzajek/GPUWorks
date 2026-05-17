#include "Interops/UE/GPUProgramObject.h"
#include "Interops/UE/GPUContextObject.h"
#include "Interops/UE/GPUBufferObject.h"
#include "Interops/UE/GPUImageObject.h"

#include "Assets/GPUProgramAsset.h"

bool UGPUProgramObject::BuildFromSource(UGPUContextObject* contextObject,
                                        const FString& source)
{
    if (!contextObject || !contextObject->GetContext())
    {
        LastBuildLog = TEXT("Invalid GPU context.");
        return false;
    }

    std::string buildLog;
    Program = contextObject->GetContext()->CreateProgramFromSource(TCHAR_TO_UTF8(*source),
                                                                   &buildLog);

    LastBuildLog = UTF8_TO_TCHAR(buildLog.c_str());
    return Program != nullptr;
}

bool UGPUProgramObject::BuildFromAsset(UGPUContextObject* contextObject,
                                       UGPUProgramAsset* asset)
{
    if (!contextObject || !contextObject->GetContext())
    {
        LastBuildLog = TEXT("Invalid GPU context.");
        return false;
    }

	FString programSource = asset->GetSourceCodeForBackend(contextObject->GetGPUBackend());

    std::string buildLog;
    Program = contextObject->GetContext()->CreateProgramFromSource(TCHAR_TO_UTF8(*programSource),
                                                                   &buildLog);

    LastBuildLog = UTF8_TO_TCHAR(buildLog.c_str());
    return Program != nullptr;
}

void UGPUProgramObject::SetKernel(const FString& kernelName)
{
    if (!Program)
    {
        return;
    }

    if (HasKernel(kernelName))
    {
        mpKernel = Program->CreateKernel(TCHAR_TO_UTF8(*kernelName));
    }
}

bool UGPUProgramObject::IsValidProgram() const
{
    return Program != nullptr;
}

FString UGPUProgramObject::GetLastBuildLog() const
{
    return LastBuildLog;
}

bool UGPUProgramObject::HasKernel(const FString& kernelName) const
{
    if (!Program)
    {
        return false;
    }

    std::shared_ptr<Gpu::IKernel> kernel = Program->CreateKernel(TCHAR_TO_UTF8(*kernelName));
    return kernel != nullptr;
}

bool UGPUProgramObject::SetIntArg(int32 index, int32 integer)
{
    if (mpKernel)
    {
		return mpKernel->SetValueArg(index, integer);
    }
    return false;
}

bool UGPUProgramObject::SetFloatArg(int32 index, float scalar)
{
    if (mpKernel)
    {
		return mpKernel->SetValueArg(index, scalar);
    }
    return false;
}

bool UGPUProgramObject::SetIntVector2Arg(int32 index, const FIntPoint& vec)
{
    if (mpKernel)
    {
        return mpKernel->SetValueArg(index, vec);
    }
    return false;
}

bool UGPUProgramObject::SetIntVector4Arg(int32 index, const FIntVector4& vec)
{
    if (mpKernel)
    {
        return mpKernel->SetValueArg(index, vec);
    }
    return false;
}

bool UGPUProgramObject::SetVector2fArg(int32 index, const FVector2f& vec)
{
    if (mpKernel)
    {
        return mpKernel->SetValueArg(index, vec);
    }
    return false;
}

bool UGPUProgramObject::SetVector4fArg(int32 index, const FVector4f& vec)
{
    if (mpKernel)
    {
        return mpKernel->SetValueArg(index, vec);
    }
    return false;
}

bool UGPUProgramObject::SetBufferArg(int32 index, UGPUBufferObject* buffer)
{
	const std::shared_ptr<Gpu::IBuffer> gpuBuffer = buffer ? buffer->GetBuffer() : nullptr;
    if (mpKernel && gpuBuffer)
    {
        
        return mpKernel->SetBufferArg(index, *gpuBuffer);
    }
    return false;
}

bool UGPUProgramObject::SetImageArg(int32 index, UGPUImageObject* image)
{
    const std::shared_ptr<Gpu::IImage> gpuImage = image ? image->GetImage() : nullptr;
    if (mpKernel && gpuImage)
    {

        return mpKernel->SetImageArg(index, *gpuImage);
    }
    return false;
}