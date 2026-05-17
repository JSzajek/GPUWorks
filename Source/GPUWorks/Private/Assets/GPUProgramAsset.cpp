#include "Assets/GPUProgramAsset.h"

#include "GPUWorksLog.h"


FString UGPUProgramAsset::GetSourceCodeForBackend(EGPUBackend backend) const
{
    switch (backend)
    {
        case EGPUBackend::OpenCL:
        {
            if (Language == EGPUProgramLanguage::OpenCL_C)
            {
                return OpenCLSource;
            }
            break;
        }
        case EGPUBackend::CUDA:
        {
            if (Language == EGPUProgramLanguage::CUDA_C)
            {
                return CUDASource;
            }
            break;
        }
        default:
            break;
    }

    UE_LOG(LogGPUWorks, Warning, TEXT("Requested backend does not match program asset language!"));
    return FString();
}

FString UGPUProgramAsset::GetSourceCodeForBackend(Gpu::Backend backend) const
{
    switch (backend)
    {
        case Gpu::Backend::OpenCL:
        {
            if (Language == EGPUProgramLanguage::OpenCL_C)
            {
                return OpenCLSource;
            }
            break;
        }
        case Gpu::Backend::CUDA:
        {
            if (Language == EGPUProgramLanguage::CUDA_C)
            {
                return CUDASource;
            }
            break;
        }
        default:
            break;
    }

    UE_LOG(LogGPUWorks, Warning, TEXT("Requested backend does not match program asset language!"));
    return FString();
}

void UGPUProgramAsset::SetSourceCodeForBackend(EGPUBackend backend,
                                               const FString& source)

{
    switch (backend)
    {
        case EGPUBackend::OpenCL:
        {
            if (Language == EGPUProgramLanguage::OpenCL_C)
            {
                OpenCLSource = source;
            }
            else
            {
                UE_LOG(LogGPUWorks, Warning, TEXT("Requested backend does not match program asset language!"));
            }
            break;
        }
        case EGPUBackend::CUDA:
        {
            if (Language == EGPUProgramLanguage::CUDA_C)
            {
                CUDASource = source;
            }
            else
            {
                UE_LOG(LogGPUWorks, Warning, TEXT("Requested backend does not match program asset language!"));
            }
            break;
        }
        default:
        {
            UE_LOG(LogGPUWorks, Warning, TEXT("Requested backend does not match program asset language!"));
            break;
        }
    }
}
