#pragma once

#include "CoreMinimal.h"
#include "GPU/GPUTypes.h"

namespace Gpu
{
    /// <summary>
	/// The hardware metrics of the GPU device.
    /// </summary>
    struct HardwareMetrics
    {
    public:
        uint32_t mComputeUnitCount = 0;
        uint64_t mGlobalMemoryBytes = 0;
        uint64_t mLocalMemoryBytes = 0;
        uint64_t mMaxWorkGroupSize = 0;
    };

    /// <summary>
	/// The kernel dispatch information, including the kernel name, global
    /// and local work sizes, and the GPU backend used for dispatch.
    /// </summary>
    struct KernelDispatchInfo
    {
    public:
        FString mName;

        uint32_t mDimensions = 1;

        uint64_t mGlobal[3] = { 1, 1, 1 };
        uint64_t mLocal[3] = { 1, 1, 1 };

        Gpu::Backend mBackend = Gpu::Backend::Unknown;
    };

    /// <summary>
	/// The static information of a kernel, including the kernel work group size,
    /// preferred work group multiple, compiled work group size, and memory usage.
    /// </summary>
    struct KernelStaticInfo
    {
    public:
        uint64_t mKernelWorkGroupSize = 0;
        uint64_t mPreferredWorkGroupMultiple = 0;

        std::array<uint64_t, 3> mCompiledWorkGroupSize = { 0, 0, 0 };

        uint64_t mPrivateMemoryBytes = 0;
        uint64_t mLocalMemoryBytes = 0;
    };

    /// <summary>
	/// The kernel profile, which includes the kernel dispatch information, static information, and timing information.
    /// </summary>
    struct KernelProfile
    {
    public:
        /// <summary>
        /// Retrieves the timing duration of the kernel execution in milliseconds.
        /// </summary>
        /// <returns>The duration in milliseconds</returns>
        float GetDurationMs() const
        {
            return mEndTimeNs > mStartTimeNs ? static_cast<float>(mEndTimeNs - mStartTimeNs) * 1.0e-6f : 0.0f;
        }

        uint64 GetWorkGroupCount() const
        {
            uint64_t count = 1;
            for (uint32_t i = 0; i < mDispatch.mDimensions; ++i)
            {
                const uint64_t LocalSize = mDispatch.mLocal[i] > 0 ? mDispatch.mLocal[i] : 1;
                count *= FMath::DivideAndRoundUp(mDispatch.mGlobal[i], LocalSize);
            }
            return count;
        }
    public:
        KernelDispatchInfo mDispatch;
        KernelStaticInfo mStaticInfo;

		uint64_t mStartTimeNs = 0;
		uint64_t mEndTimeNs = 0;

        bool mIsComplete = false;
        bool mIsValidTiming = false;
    };
}