#pragma once

#include <functional>

namespace Gpu
{
    class IEvent
    {
    public:
        using Callback = std::function<void()>;

        virtual ~IEvent() = default;

        virtual bool IsComplete() const = 0;
        virtual void Wait() = 0;
        virtual void SetCompletionCallback(Callback&& callback) = 0;
    };
}