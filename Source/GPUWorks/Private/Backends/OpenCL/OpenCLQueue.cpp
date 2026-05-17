#include "Backends/OpenCL/OpenCLQueue.h"
#include "Backends/OpenCL/OpenCLKernel.h"
#include "Backends/OpenCL/OpenCLEvent.h"
#include "Backends/OpenCL/OpenCLDevice.h"
#include "Backends/OpenCL/OpenCLContext.h"

#include "Profiler/IGPUProfilerBackend.h"
#include "Profiler/GPUProfilerManager.h"

namespace Gpu::OpenCL
{
    Queue::Queue(std::shared_ptr<Context> inContext,
                 cl_command_queue inQueue)
        : mpContext(inContext),
        mpQueueHandle(inQueue)
    {
    }

    Queue::~Queue()
    {
        if (mpQueueHandle)
        {
            clReleaseCommandQueue(mpQueueHandle);
            mpQueueHandle = nullptr;
        }
    }

    void Queue::Flush()
    {
        if (mpQueueHandle)
        {
            clFlush(mpQueueHandle);
        }
    }

    void Queue::Finish()
    {
        if (mpQueueHandle)
        {
            clFinish(mpQueueHandle);
        }
    }

    std::shared_ptr<IEvent> Queue::Dispatch(IKernel& kernelBase,
                                            const DispatchDescription& desc)
    {
        OpenCL::Kernel* kernel = reinterpret_cast<OpenCL::Kernel*>(&kernelBase);
        if (!kernel || !mpQueueHandle)
        {
            return nullptr;
        }

        const size_t global[3] = { desc.Global[0], desc.Global[1], desc.Global[2] };
        const size_t local[3] = { desc.Local[0], desc.Local[1], desc.Local[2] };
        const size_t* localPtr = (local[0] == 0) ? nullptr : local;

        cl_event outEvent = nullptr;
        const cl_int err = clEnqueueNDRangeKernel(mpQueueHandle,
                                                  kernel->GetCLKernel(),
                                                  static_cast<cl_uint>(desc.Dim),
                                                  nullptr,
                                                  global,
                                                  localPtr,
                                                  0,
                                                  nullptr,
                                                  &outEvent);

    #if GPUWORKS_PROFILING
        OpenCL::Device* attachedDevice = nullptr;
        std::shared_ptr<OpenCL::Context> contextPtr = mpContext.lock();
        if (contextPtr)
        {
            attachedDevice = reinterpret_cast<OpenCL::Device*>(contextPtr->GetDevice().get());
        }

        Gpu::ProfiledKernelHandle Handle;
        Handle.Backend = Gpu::Backend::OpenCL;
        Handle.NativeEvent = outEvent;
        Handle.NativeKernel = kernel->GetCLKernel();
        Handle.NativeDevice = attachedDevice->GetCLDevice();

        Gpu::KernelDispatchInfo Dispatch;
        Dispatch.Name = FString(UTF8_TO_TCHAR(kernel->GetName().c_str()));
        Dispatch.Backend = Gpu::Backend::OpenCL;
        Dispatch.Dim = static_cast<uint32>(desc.Dim);

        for (uint32 i = 0; i < Dispatch.Dim; ++i)
        {
            Dispatch.Global[i] = global[i];
            Dispatch.Local[i] = local[i];
        }

        FGPUProfilerManager::EnqueueProfiledKernel(Handle, Dispatch);
    #endif

        if (err != CL_SUCCESS)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dispatch Error: %s"), *FString(GetErrorString(err).c_str()));
            return nullptr;
        }
        return std::make_shared<Event>(outEvent);
    }
}