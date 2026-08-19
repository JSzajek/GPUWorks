#pragma once

#include <cstdint>
#include <memory>

namespace Gpu
{
    class IKernel;
    class IEvent;

    /// <summary>
	/// Dispatch description for a kernel dispatch.
    /// </summary>
    struct DispatchDescription
    {
        uint32_t mDim = 1;
        size_t mGlobal[3] = { 1, 1, 1 };
        size_t mLocal[3] = { 0, 0, 0 }; // 0 means backend chooses
    };

    /// <summary>
	/// Interface for a GPU command queue.
    /// </summary>
    class IQueue
    {
    public:
        /// <summary>
        /// Default destructor.
        /// </summary>
        virtual ~IQueue() = default;

        /// <summary>
		/// Flushes the command queue, ensuring that all 
        /// previously submitted commands are issued to the GPU for execution
        /// </summary>
        virtual void Flush() = 0;

        /// <summary>
		/// Finishes the command queue, blocking until all 
        /// previously submitted commands have completed execution on the GPU.
        /// </summary>
        virtual void Finish() = 0;

        /// <summary>
		/// Dispatches a kernel to the GPU with the specified dispatch description.
        /// </summary>
        /// <param name="kernel">The kernel to be dispatched.</param>
        /// <param name="desc">The dispatch description specifying the execution configuration.</param>
        /// <returns>An event representing the completion of the kernel execution.</returns>
        virtual std::shared_ptr<IEvent> Dispatch(IKernel& kernel,
                                                 const DispatchDescription& desc) = 0;
    };
}