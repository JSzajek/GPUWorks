#pragma once

#include "GpuCore.h"
#include "GpuTypes.h"

#include <memory>

namespace Gpu
{
    struct FactoryDesc
    {
        Backend PreferredBackend = Backend::OpenCL;
        bool bAllowFallback = true;
        int32_t PreferredDeviceIndex = 0;
    };

    class GPUWORKS_API Factory
    {
    public:
        // Create a backend core explicitly.
        static std::shared_ptr<ICore> Create(Backend backend);

        // Create using preference + optional fallback.
        static std::shared_ptr<ICore> Create(const FactoryDesc& desc);

        // Utility helpers
        static bool IsBackendAvailable(Backend backend);
        static std::vector<Backend> GetAvailableBackends();

    private:
        static std::shared_ptr<ICore> CreateOpenCL();
        static std::shared_ptr<ICore> CreateCUDA();
    };
}