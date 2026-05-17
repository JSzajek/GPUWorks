#include "Backends/OpenCL/OpenCLEvent.h"

namespace Gpu::OpenCL
{
    Event::Event(cl_event inEvent)
        : EventHandle(inEvent),
        mpState(std::make_shared<CallbackState>())
    {
    }

    Event::~Event()
    {
        Reset(nullptr);
    }

	Event::Event(Event&& Other) noexcept
	{
        EventHandle = Other.EventHandle;
        mpState = std::move(Other.mpState);

        Other.EventHandle = nullptr;
        Other.mpState = std::make_shared<CallbackState>();
	}

    Event& Event::operator=(Event&& Other) noexcept
    {
        if (this != &Other)
        {
            Reset(nullptr);

            EventHandle = Other.EventHandle;
            mpState = std::move(Other.mpState);

            Other.EventHandle = nullptr;
            Other.mpState = std::make_shared<CallbackState>();
        }
        return *this;
    }

    void Event::Reset(cl_event inEvent)
    {
        if (EventHandle)
        {
            clReleaseEvent(EventHandle);
        }

        EventHandle = inEvent;

        if (!mpState)
        {
            mpState = std::make_shared<CallbackState>();
        }
    }

    cl_event Event::Detach()
    {
        cl_event Out = EventHandle;
        EventHandle = nullptr;
        return Out;
    }


    bool Event::IsComplete() const
    {
        if (!EventHandle)
        {
            return true;
        }

        cl_int status = 0;
        if (clGetEventInfo(EventHandle,
                           CL_EVENT_COMMAND_EXECUTION_STATUS,
                           sizeof(status),
                           &status,
                           nullptr) != CL_SUCCESS)
        {
            return false;
        }
        return status == CL_COMPLETE;
    }

    void Event::Wait()
    {
        if (EventHandle)
        {
            clWaitForEvents(1, &EventHandle);
        }
    }

    void Event::SetCompletionCallback(Callback&& callback)
    {
        if (!callback)
        {
            return;
        }

        if (!mpState)
        {
            mpState = std::make_shared<CallbackState>();
        }

        // No native event means there is nothing to wait for.
        if (!EventHandle)
        {
            callback();
            return;
        }

        // If already complete, run immediately.
        if (IsComplete())
        {
            callback();
            return;
        }

        {
            const std::scoped_lock Lock(mpState->mMutex);
            mpState->mCompletionCallback = std::move(callback);
        }

        // Keep the callback state alive independently of this Event object.
        auto* StateCopy = new std::shared_ptr<CallbackState>(mpState);

        const cl_int Err = clSetEventCallback(
            EventHandle,
            CL_COMPLETE,
            &Event::OnComplete,
            StateCopy);

        if (Err != CL_SUCCESS)
        {
            std::unique_ptr<std::shared_ptr<CallbackState>> Cleanup(StateCopy);
            Callback CallbackToRun;
            {
                const std::scoped_lock Lock(mpState->mMutex);
                CallbackToRun = std::move(mpState->mCompletionCallback);
            }

            if (CallbackToRun)
            {
                CallbackToRun();
            }
        }
    }

    void CL_CALLBACK Event::OnComplete(cl_event handle,
                                       cl_int status,
                                       void* userData)
    {
        std::unique_ptr<std::shared_ptr<CallbackState>> StateOwner(static_cast<std::shared_ptr<CallbackState>*>(userData));

        if (!StateOwner || !(*StateOwner))
        {
            return;
        }

        std::shared_ptr<CallbackState> LocalState = *StateOwner;
        bool bExpected = false;
        if (!LocalState->mCallbackFired.compare_exchange_strong(bExpected,
                                                                true,
                                                                std::memory_order_acq_rel))
        {
            return;
        }

        Callback CallbackToRun;
        {
            std::scoped_lock Lock(LocalState->mMutex);
            CallbackToRun = std::move(LocalState->mCompletionCallback);
        }

        if (CallbackToRun)
        {
            CallbackToRun();
        }

        // Important:
        // Do NOT call clReleaseEvent(EventHandle) here.
        // The Event object owns and releases the cl_event.
    }
}