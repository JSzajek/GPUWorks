#pragma once

#include <cstdint>
#include <memory>

namespace Gpu
{
    class IKernel;
    class IEvent;

    struct DispatchDescription
    {
        uint32_t Dim = 1;
        size_t Global[3] = { 1, 1, 1 };
        size_t Local[3] = { 0, 0, 0 }; // 0 means backend chooses
    };

    class IQueue
    {
    public:
        virtual ~IQueue() = default;

        virtual void Flush() = 0;
        virtual void Finish() = 0;
        virtual std::shared_ptr<IEvent> Dispatch(IKernel& kernel, const DispatchDescription& desc) = 0;
    };
}