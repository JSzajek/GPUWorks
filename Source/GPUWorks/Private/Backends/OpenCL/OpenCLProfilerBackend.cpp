#include "Backends/OpenCL/OpenCLProfilerBackend.h"

#include "GPU/GPUFactory.h"
#include "Backends/OpenCL/OpenCLDevice.h"

#include "OpenCLLib.h"

namespace Gpu::OpenCL
{
    bool ProfilerBackend::RetainProfiledHandle(ProfiledKernelHandle& Handle)
    {
        cl_event Event = static_cast<cl_event>(Handle.NativeEvent);
        if (!Event)
        {
            return false;
        }

        return clRetainEvent(Event) == CL_SUCCESS;
    }

    void ProfilerBackend::ReleaseProfiledHandle(ProfiledKernelHandle& Handle)
    {
        cl_event Event = static_cast<cl_event>(Handle.NativeEvent);
        if (Event)
        {
            clReleaseEvent(Event);
            Handle.NativeEvent = nullptr;
        }
    }

    bool ProfilerBackend::IsComplete(const ProfiledKernelHandle& Handle)
    {
        cl_event _event = static_cast<cl_event>(Handle.NativeEvent);
        if (!_event)
        {
            return true;
        }

        cl_int status = 0;
        const cl_int err = clGetEventInfo(_event,
                                          CL_EVENT_COMMAND_EXECUTION_STATUS,
                                          sizeof(status),
                                          &status,
                                          nullptr);

        return err == CL_SUCCESS && status == CL_COMPLETE;
    }

    bool ProfilerBackend::QueryTiming(const ProfiledKernelHandle& Handle,
                                      uint64_t& OutStartTimeNs,
                                      uint64_t& OutEndTimeNs)
    {
        cl_event Event = static_cast<cl_event>(Handle.NativeEvent);
        if (!Event)
        {
            return false;
        }

        cl_ulong Start = 0;
        cl_ulong End = 0;

        if (clGetEventProfilingInfo(Event, CL_PROFILING_COMMAND_START, sizeof(Start), &Start, nullptr) != CL_SUCCESS)
        {
            return false;
        }

        if (clGetEventProfilingInfo(Event, CL_PROFILING_COMMAND_END, sizeof(End), &End, nullptr) != CL_SUCCESS)
        {
            return false;
        }

        OutStartTimeNs = static_cast<uint64>(Start);
        OutEndTimeNs = static_cast<uint64>(End);
        return End >= Start;
    }

    bool ProfilerBackend::QueryKernelStaticInfo(const ProfiledKernelHandle& Handle,
                                                KernelStaticInfo& OutInfo)
    {
        cl_kernel Kernel = static_cast<cl_kernel>(Handle.NativeKernel);
        cl_device_id Device = static_cast<cl_device_id>(Handle.NativeDevice);

        if (!Kernel || !Device)
        {
            return false;
        }

        clGetKernelWorkGroupInfo(Kernel,
                                 Device,
                                 CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(size_t),
                                 &OutInfo.KernelWorkGroupSize,
                                 nullptr);

        clGetKernelWorkGroupInfo(Kernel,
                                 Device,
                                 CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                 sizeof(size_t),
                                 &OutInfo.PreferredWorkGroupMultiple,
                                 nullptr);

        clGetKernelWorkGroupInfo(Kernel,
                                 Device,
                                 CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                                 sizeof(OutInfo.CompiledWorkGroupSize),
                                 &OutInfo.CompiledWorkGroupSize,
                                 nullptr);

        cl_ulong PrivateMem = 0;
        cl_ulong LocalMem = 0;

        clGetKernelWorkGroupInfo(Kernel,
                                 Device,
                                 CL_KERNEL_PRIVATE_MEM_SIZE,
                                 sizeof(PrivateMem),
                                 &PrivateMem,
                                 nullptr);

        clGetKernelWorkGroupInfo(Kernel,
                                 Device,
                                 CL_KERNEL_LOCAL_MEM_SIZE,
                                 sizeof(LocalMem),
                                 &LocalMem,
                                 nullptr);

        OutInfo.PrivateMemoryBytes = static_cast<uint64>(PrivateMem);
        OutInfo.LocalMemoryBytes = static_cast<uint64>(LocalMem);

        return true;
    }

    HardwareMetrics ProfilerBackend::QueryHardwareMetrics()
    {
        Gpu::FactoryDesc desc;
        desc.PreferredBackend = Gpu::Backend::OpenCL;
        desc.bAllowFallback = false;
        
        std::shared_ptr<Gpu::ICore> corePtr = Gpu::Factory::Create(desc);
        std::shared_ptr<Gpu::OpenCL::Device> devicePtr = reinterpret_pointer_cast<Gpu::OpenCL::Device>(corePtr->GetDevice(0));

        HardwareMetrics metrics;
        cl_device_id device = devicePtr->GetCLDevice();
        if (!device)
        {
            return metrics;
        }

        cl_uint computeUnits = 0;
        size_t maxWorkGroupSize = 0;
        cl_ulong globalMem = 0;
        cl_ulong localMem = 0;

        clGetDeviceInfo(device,
                        CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(computeUnits),
                        &computeUnits,
                        nullptr);

        clGetDeviceInfo(device,
                        CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(maxWorkGroupSize),
                        &maxWorkGroupSize,
                        nullptr);

        clGetDeviceInfo(device,
                        CL_DEVICE_GLOBAL_MEM_SIZE,
                        sizeof(globalMem),
                        &globalMem,
                        nullptr);

        clGetDeviceInfo(device,
                        CL_DEVICE_LOCAL_MEM_SIZE,
                        sizeof(localMem),
                        &localMem,
                        nullptr);

        metrics.mComputeUnitCount = computeUnits;
        metrics.mMaxWorkGroupSize = static_cast<uint64>(maxWorkGroupSize);
        metrics.mGlobalMemoryBytes = static_cast<uint64>(globalMem);
        metrics.mLocalMemoryBytes = static_cast<uint64>(localMem);

        return metrics;
    }
}