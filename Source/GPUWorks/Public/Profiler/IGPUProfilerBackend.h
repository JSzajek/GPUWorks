#pragma once

#include "Profiler/GPUProfilerTypes.h"

namespace Gpu
{
    struct ProfiledKernelHandle
    {
        Gpu::Backend Backend = Backend::Unknown;
        void* NativeEvent = nullptr;
        void* NativeKernel = nullptr;
        void* NativeDevice = nullptr;
    };

    class IProfilerBackend
    {
    public:
        virtual ~IProfilerBackend() = default;

        virtual Backend GetBackend() const = 0;

        virtual bool RetainProfiledHandle(ProfiledKernelHandle& Handle) = 0;
        virtual void ReleaseProfiledHandle(ProfiledKernelHandle& Handle) = 0;

        virtual bool IsComplete(const ProfiledKernelHandle& Handle) = 0;

        virtual bool QueryTiming(const ProfiledKernelHandle& Handle,
                                 uint64& OutStartTimeNs,
                                 uint64& OutEndTimeNs) = 0;

        virtual bool QueryKernelStaticInfo(const ProfiledKernelHandle& Handle,
                                           KernelStaticInfo& OutInfo) = 0;

        virtual HardwareMetrics QueryHardwareMetrics() = 0;
    };
}