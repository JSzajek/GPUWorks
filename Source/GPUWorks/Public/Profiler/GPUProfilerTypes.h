#pragma once

#include "CoreMinimal.h"
#include "GPU/GPUTypes.h"

namespace Gpu
{
    struct HardwareMetrics
    {
    public:
        uint32_t mComputeUnitCount = 0;
        uint64_t mGlobalMemoryBytes = 0;
        uint64_t mLocalMemoryBytes = 0;
        uint64_t mMaxWorkGroupSize = 0;
    };

    struct KernelDispatchInfo
    {
    public:
        FString Name;

        uint32_t Dim = 1;

        uint64_t Global[3] = { 1, 1, 1 };
        uint64_t Local[3] = { 1, 1, 1 };

        Gpu::Backend Backend = Gpu::Backend::Unknown;
    };

    struct KernelStaticInfo
    {
    public:
        uint64_t KernelWorkGroupSize = 0;
        uint64_t PreferredWorkGroupMultiple = 0;

        uint64_t CompiledWorkGroupSize[3] = { 0, 0, 0 };

        uint64_t PrivateMemoryBytes = 0;
        uint64_t LocalMemoryBytes = 0;
    };

    struct KernelProfile
    {
    public:
        float GetDurationMs() const
        {
            return EndTimeNs > StartTimeNs ? static_cast<float>(EndTimeNs - StartTimeNs) * 1.0e-6f : 0.0f;
        }

        uint64 GetWorkGroupCount() const
        {
            uint64 count = 1;
            for (uint32 i = 0; i < Dispatch.Dim; ++i)
            {
                const uint64 LocalSize = Dispatch.Local[i] > 0 ? Dispatch.Local[i] : 1;
                count *= FMath::DivideAndRoundUp(Dispatch.Global[i], LocalSize);
            }
            return count;
        }
    public:
        KernelDispatchInfo Dispatch;
        KernelStaticInfo StaticInfo;

		uint64_t StartTimeNs = 0;
		uint64_t EndTimeNs = 0;

        bool bComplete = false;
        bool bValidTiming = false;
    };
}