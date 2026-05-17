#include "Public/GPU/GpuFactory.h"

#include "Backends/OpenCL/OpenCLCore.h"

// Add later when ready
// #include "Backends/CUDA/CUDACore.h"

namespace Gpu
{
    std::shared_ptr<ICore> Factory::Create(Backend backend)
    {
        switch (backend)
        {
            case Backend::OpenCL:
                return CreateOpenCL();

            case Backend::CUDA:
                return CreateCUDA();
            default:
                return nullptr;
        }
    }

    std::shared_ptr<ICore> Factory::Create(const FactoryDesc& desc)
    {
        if (auto core = Create(desc.PreferredBackend))
        {
            return core;
        }

        if (!desc.bAllowFallback)
        {
            return nullptr;
        }

        for (Backend backend : GetAvailableBackends())
        {
            if (backend == desc.PreferredBackend)
            {
                continue;
            }

            if (auto core = Create(backend))
            {
                return core;
            }
        }

        return nullptr;
    }

    bool Factory::IsBackendAvailable(Backend backend)
    {
        return Create(backend) != nullptr;
    }

    std::vector<Backend> Factory::GetAvailableBackends()
    {
        std::vector<Backend> out;

        if (CreateOpenCL())
        {
            out.push_back(Backend::OpenCL);
        }

        if (CreateCUDA())
        {
            out.push_back(Backend::CUDA);
        }

        return out;
    }

    std::shared_ptr<ICore> Factory::CreateOpenCL()
    {
        return OpenCL::Core::TryCreate();
    }

    std::shared_ptr<ICore> Factory::CreateCUDA()
    {
        // Placeholder until CUDA backend is implemented.
        // return CUDA::Core::TryCreate();
        return nullptr;
    }
}