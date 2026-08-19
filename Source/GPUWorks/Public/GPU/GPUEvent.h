#pragma once

#include <functional>

namespace Gpu
{
    /// <summary>
	/// Interface for GPU events, which can be used to synchronize GPU and CPU operations.
    /// </summary>
    class IEvent
    {
    public:
        using Callback = std::function<void()>;

        /// <summary>
        /// Default destructor.
        /// </summary>
        virtual ~IEvent() = default;

        /// <summary>
		/// Retrieves whether the event has completed or not.
        /// </summary>
		/// <returns>True if the event has completed, false otherwise</returns>
        virtual bool IsComplete() const = 0;

        /// <summary>
		/// Waits for the event to complete.
        /// 
        /// NOTE: This is a blocking command.
        /// </summary>
        virtual void Wait() = 0;

        /// <summary>
		/// Sets the callback function to be called when the event is completed.
        /// </summary>
        /// <param name="callback">The callback function to be called when the event is completed</param>
        virtual void SetCompletionCallback(Callback&& callback) = 0;
    };
}