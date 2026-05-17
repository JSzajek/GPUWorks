#pragma once

#include <cstdint>

namespace Gpu
{
    enum class SVMSupport : uint8_t
    {
        None = 0,
        Coarse,
        Fine,
    };

    struct Capabilities
    {
        bool bSupportsImages = false;
        bool bSupportHalfFloat = false;
        bool bSupportsRuntimeCompilation = false;
        bool bSupportsUnifiedMemory = false;
        bool bSupportsEvents = true;
        bool bSupportsInteropWithD3D = false;
        bool bSupportsInteropWithVulkan = false;

        uint32_t MaxWorkGroupSize = 0;
        uint32_t ComputeUnitCount = 0;
        uint64_t GlobalMemoryBytes = 0;

        SVMSupport mSVMSupport = SVMSupport::None;
    };
}