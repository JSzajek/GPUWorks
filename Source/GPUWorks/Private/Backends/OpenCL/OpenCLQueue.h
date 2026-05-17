#pragma once

#include "GPU/GPUQueue.h"
#include "Backends/OpenCL/OpenCLCommon.h"

namespace Gpu::OpenCL
{
    class Context;
    class Kernel;
    class Event;

    class Queue final : public IQueue,
                        public std::enable_shared_from_this<Queue>
    {
    public:
        Queue(std::shared_ptr<Context> inContext,
              cl_command_queue inQueue);

        virtual ~Queue() override;
    public:
        virtual void Flush() override;
        virtual void Finish() override;
        virtual std::shared_ptr<IEvent> Dispatch(IKernel& kernel,
                                                 const DispatchDescription& desc) override;

        cl_command_queue GetCLQueue() const { return mpQueueHandle; }

    private:
        std::weak_ptr<Context> mpContext;
        cl_command_queue mpQueueHandle = nullptr;
    };

    
}