#pragma once

#include "Profiler/IGpuProfilerBackend.h"

namespace Gpu::OpenCL
{
    /// <summary>
	/// OpenCL implementation of the GPU profiler backend interface.
    /// </summary>
    class ProfilerBackend final : public IProfilerBackend
    {
    public:
        /// <summary>
        /// Retrieves the GPU backend type associated with this profiler backend.
        /// </summary>
        /// <returns>The backend type</returns>
        inline virtual Backend GetBackend() const override { return Backend::OpenCL; }

        /// <summary>
        /// Retains a profiled kernel handle for querying profiling results.
        /// The backend may need to retain the underlying
        /// </summary>
        /// <param name="handle">The kernel handle</param>
        /// <returns>True if the operation was successful</returns>
        virtual bool RetainProfiledHandle(ProfiledKernelHandle& handle) override;

        /// <summary>
        /// Releases a profiled kernel handle after profiling results have been retrieved.
        /// </summary>
        /// <param name="handle">The kernel handle</param>
        virtual void ReleaseProfiledHandle(ProfiledKernelHandle& handle) override;

        /// <summary>
        /// Retrieves whether the profiled kernel dispatch associated 
        /// with the given handle has completed execution on the GPU.
        /// </summary>
        /// <param name="handle">The kernel handle</param>
        /// <returns>True if the kernel is complete</returns>
        virtual bool IsComplete(const ProfiledKernelHandle& handle) override;

        /// <summary>
        /// Queries the start and end timestamps of the kernel execution associated with the given handle.
        /// </summary>
        /// <param name="handle">The kernel handle</param>
        /// <param name="startTimeNs">The start timestamp in nanoseconds</param>
        /// <param name="endTimeNs">The end timestamp in nanoseconds</param>
        /// <returns>True if the query was successful</returns>
        virtual bool QueryTiming(const ProfiledKernelHandle& handle,
                                 uint64_t& startTimeNs,
                                 uint64_t& endTimeNs) override;

        /// <summary>
        /// Queries the kernel static information query function retrieves static information about a kernel dispatch.
        /// </summary>
        /// <param name="handle">The kernel handle</param>
        /// <param name="outInfo">The output structure to receive the kernel static information</param>
        /// <returns>True if the query was successful</returns>
        virtual bool QueryKernelStaticInfo(const ProfiledKernelHandle& handle,
                                           KernelStaticInfo& outInfo) override;

        /// <summary>
        /// Queries the hardware metrics of the GPU device associated with this profiler backend.
        /// </summary>
        /// <returns>The hardware metrics</returns>
        virtual HardwareMetrics QueryHardwareMetrics() override;
    };
}