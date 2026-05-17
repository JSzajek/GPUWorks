#pragma once

#include "Profiler/IGpuProfilerBackend.h"

namespace Gpu::OpenCL
{
    class ProfilerBackend final : public IProfilerBackend
    {
    public:
        virtual Backend GetBackend() const override { return Backend::OpenCL; }

        virtual bool RetainProfiledHandle(ProfiledKernelHandle& Handle) override;
        virtual void ReleaseProfiledHandle(ProfiledKernelHandle& Handle) override;

        virtual bool IsComplete(const ProfiledKernelHandle& Handle) override;

        virtual bool QueryTiming(const ProfiledKernelHandle& Handle,
                                 uint64_t& OutStartTimeNs,
                                 uint64_t& OutEndTimeNs) override;

        virtual bool QueryKernelStaticInfo(const ProfiledKernelHandle& Handle,
                                           KernelStaticInfo& OutInfo) override;

        virtual HardwareMetrics QueryHardwareMetrics() override;
    };
}