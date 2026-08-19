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

        const size_t global[3] = { desc.mGlobal[0], desc.mGlobal[1], desc.mGlobal[2] };
        const size_t local[3] = { desc.mLocal[0], desc.mLocal[1], desc.mLocal[2] };
        const size_t* localPtr = (local[0] == 0) ? nullptr : local;

        cl_event outEvent = nullptr;
        const cl_int err = clEnqueueNDRangeKernel(mpQueueHandle,
                                                  kernel->GetCLKernel(),
                                                  static_cast<cl_uint>(desc.mDim),
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
        Handle.mBackend = Gpu::Backend::OpenCL;
        Handle.mpNativeEvent = outEvent;
        Handle.mpNativeKernel = kernel->GetCLKernel();
        Handle.mpNativeDevice = attachedDevice->GetCLDevice();

        Gpu::KernelDispatchInfo Dispatch;
        Dispatch.mName = FString(UTF8_TO_TCHAR(kernel->GetName().c_str()));
        Dispatch.mBackend = Gpu::Backend::OpenCL;
        Dispatch.mDimensions = static_cast<uint32>(desc.mDim);

        for (uint32 i = 0; i < Dispatch.mDimensions; ++i)
        {
            Dispatch.mGlobal[i] = global[i];
            Dispatch.mLocal[i] = local[i];
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