#pragma once

#include <cstdint>

namespace Gpu
{
    /// <summary>
	/// Backend types for GPU execution.
    /// </summary>
    enum class Backend : uint8_t
    {
        Unknown = 0,
        OpenCL,
        CUDA,
    };

    /// <summary>
	/// Access types for GPU resources.
    /// </summary>
    enum class Access : uint8_t
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    /// <summary>
	/// Memory usage types for GPU resources.
    /// </summary>
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

    /// <summary>
	/// Buffer synchronization modes for GPU resources.
    /// </summary>
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

    /// <summary>
	/// Program formats for GPU kernels.
    /// </summary>
    enum class ProgramFormat : uint8_t
    {
        Source,
        Intermediate,   // e.g. PTX / SPIR-V later
        Binary
    };

    /// <summary>
	/// Image types for GPU resources.
    /// </summary>
    enum class ImageType : uint8_t
    {
        Tex2D,
        Tex2DArray,
        Tex3D,
    };

    /// <summary>
	/// Pixel formats for GPU resources.
    /// </summary>
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