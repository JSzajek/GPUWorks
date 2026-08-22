#pragma once

#include <cstdint>

namespace Gpu
{
    /// <summary>
	/// Enum representing the level of support for Shared Virtual Memory (SVM) in a GPU device.
    /// </summary>
    enum class SVMSupport : uint8_t
    {
        None = 0,
        Coarse,
        Fine,
    };

    /// <summary>
	/// Struct representing the capabilities of a GPU device.
    /// </summary>
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