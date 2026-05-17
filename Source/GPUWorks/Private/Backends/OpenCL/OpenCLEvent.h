#pragma once

#include "GPU/GPUEvent.h"
#include "Backends/OpenCL/OpenCLCommon.h"

#include <mutex>

namespace Gpu::OpenCL
{
    class Event final : public IEvent
    {
    public:
        explicit Event(cl_event inEvent = nullptr);
        virtual ~Event() override;

        Event(Event&& Other) noexcept;
        Event& operator=(Event&& Other) noexcept;

        virtual bool IsComplete() const override;
        virtual void Wait() override;
        virtual void SetCompletionCallback(Callback&& callback) override;

        /// <summary>
        /// Takes ownership of a new event. Releases the old one.
        /// </summary>
        /// <param name="inEvent"></param>
        void Reset(cl_event inEvent);

        /// <summary>
        /// Releases ownership without clReleaseEvent().
        /// Use only when another owner will manage the event lifetime.
        /// </summary>
        /// <returns></returns>
        cl_event Detach();

        cl_event GetCLEvent() const { return EventHandle; }
    private:
        struct CallbackState
        {
            std::mutex mMutex;
            Callback mCompletionCallback;
            std::atomic<bool> mCallbackFired = false;
        };

        static void CL_CALLBACK OnComplete(cl_event handle,
                                           cl_int status,
                                           void* userData);

        cl_event EventHandle = nullptr;
        std::shared_ptr<CallbackState> mpState = nullptr;
    };
}