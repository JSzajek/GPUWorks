#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Gpu
{
    enum class Backend : uint8_t
    {
        Unknown = 0,
        OpenCL,
        CUDA,
    };

    enum class Access : uint8_t
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    enum class MemoryUsage : uint8_t
    {
        // device-local / regular device allocation
        Default,

        // host -> device friendly
        Upload,

        // device -> host friendly
        Readback,

        // zero-copy / unified / mapped where supported
        Shared,
    };

    enum class BufferSyncMode : uint8_t
    {
        // Backend will decide the best synchronization method
		Auto,

        // Immutable after initial creation / staged read-back
        CopyOnce,

		// Frequent CPU-GPU updates via map/unmap or equivalent
        Stream,

		// Shared/SVM/mapped if supported, otherwise behaves like Stream
        ZeroCopy
    };

    enum class ProgramFormat : uint8_t
    {
        Source,
        Intermediate,   // e.g. PTX / SPIR-V later
        Binary
    };

    enum class ImageType : uint8_t
    {
        Tex2D,
        Tex2DArray,
        Tex3D,
    };

    enum class PixelFormat : uint32_t
    {
        Unknown = 0,
        R8,
        RG8,
        RGBA8,
        R16F,
        RG16F,
        RGBA16F,
        R32F,
        RG32F,
        RGBA32F,
        R32U,
        RG32U,
        RGBA32U,
        R32S,
    };
}