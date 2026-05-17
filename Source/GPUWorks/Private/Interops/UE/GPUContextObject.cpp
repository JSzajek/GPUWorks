#include "Interops/UE/GPUContextObject.h"

#include "GPU/GPUDevice.h"

bool UGPUContextObject::Initialize(EGpuBackend backend,
                                   int32 deviceIndex)
{
    const Gpu::Backend nativeBackend = (backend == EGpuBackend::OpenCL) ? Gpu::Backend::OpenCL : Gpu::Backend::CUDA;

    mpCore = Gpu::Factory::Create(nativeBackend);
    if (!mpCore)
    {
        return false;
    }

    mpDevice = mpCore->GetDevice(static_cast<uint32>(deviceIndex));
    if (!mpDevice)
    {
        mpCore.reset();
        return false;
    }

    mpContext = mpCore->CreateContext(mpDevice);
    if (!mpContext)
    {
        mpDevice.reset();
        mpCore.reset();
        return false;
    }

    mpDefaultQueue = mpContext->CreateQueue();
    return mpDefaultQueue != nullptr;
}

bool UGPUContextObject::IsValidContext() const
{
    return mpContext != nullptr;
}

FString UGPUContextObject::GetDeviceName() const
{
    if (!mpDevice)
    {
        return TEXT("Invalid");
    }

    return FString(mpDevice->GetName().c_str());
}

bool UGPUContextObject::CreateDefaultQueue()
{
    if (!mpContext)
    {
        return false;
    }

    mpDefaultQueue = mpContext->CreateQueue();
    return mpDefaultQueue != nullptr;
}

bool UGPUContextObject::HasImageSupport() const
{
    if (!mpContext)
    {
        return false;
    }

	return mpContext->GetDevice()->GetCapabilities().bSupportsImages;
}
