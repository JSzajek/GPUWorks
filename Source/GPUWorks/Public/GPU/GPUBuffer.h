#pragma once

#include "GPU/GPUTypes.h"
#include "GPU/GPUContext.h"

namespace Gpu
{
    class IQueue;
    class IKernel;
    class IEvent;

    /// <summary>
	/// Interface for GPU buffer objects.
    /// </summary>
    class IBuffer
    {
    public:
        /// <summary>
		/// Default destructor.
        /// </summary>
        virtual ~IBuffer() = default;

        /// <summary>
		/// Gets the backend type of the buffer.
        /// </summary>
        /// <returns>The backend type of the buffer.</returns>
        virtual Backend GetBackend() const = 0;

        /// <summary>
		/// Gets the size of the buffer in bytes.
        /// </summary>
        /// <returns>The size of the buffer</returns>
        virtual size_t GetSize() const = 0;

        /// <summary>
		/// Gets the description of the buffer, including its size, access mode, usage, and synchronization mode.
        /// </summary>
        /// <returns>The buffer description</returns>
        virtual const BufferDescription& GetDescription() const = 0;

        /// <summary>
		/// Checks whether the buffer can be uploaded 
        /// after creation based on its access mode and usage.
        /// </summary>
        /// <returns>True if the buffer can be uploaded to</returns>
        virtual bool CanUploadAfterCreate() const = 0;

        /// <summary>
		/// Gets the resolved synchronization mode of the 
        /// buffer after considering backend capabilities and fallbacks.
        /// </summary>
        /// <returns>The resolved synchronization mode</returns>
        virtual BufferSyncMode GetResolvedSyncMode() const = 0;

        /// <summary>
		/// Attaches the buffer to the specified kernel at the given argument index.
        /// </summary>
        /// <param name="kernel">The kernel to attach the buffer to</param>
        /// <param name="argIndex">The argument index in the kernel</param>
        /// <returns>True if the buffer was successfully attached, false otherwise</returns>
        virtual bool AttachToKernel(IKernel& kernel,
                                    uint32_t argIndex) = 0;

        /// <summary>
		/// Uploads data from the specified source to the buffer using the provided queue.
        /// </summary>
        /// <param name="queue">The queue to use for the copy operation</param>
        /// <param name="src">The source buffer to upload data to</param>
        /// <param name="bytes">The number of bytes to upload</param>
        /// <param name="offset">The offset in the buffer to start uploading from</param>
        /// <returns>True if the copy operation was successful, false otherwise</returns>
        virtual bool Upload(IQueue& queue,
                            const void* src,
                            size_t bytes,
                            size_t offset = 0) = 0;

        /// <summary>
		/// Downloads data from the buffer to the specified destination using the provided queue.
        /// </summary>
        /// <param name="queue">The queue to use for the copy operation</param>
        /// <param name="dst">The destination buffer to download data to</param>
        /// <param name="bytes">The number of bytes to download</param>
        /// <param name="offset">The offset in the buffer to start downloading from</param>
        /// <returns>True if the copy operation was successful, false otherwise</returns>
        virtual bool Download(IQueue& queue,
                              void* dst,
                              size_t bytes,
                              size_t offset = 0) = 0;

        /// <summary>
		/// Uploads data from the specified source to the buffer asynchronously using the provided queue.
        /// </summary>
        /// <param name="queue">The queue to use for the copy operation</param>
        /// <param name="src">The source buffer to upload data to</param>
        /// <param name="bytes">The number of bytes to upload</param>
        /// <param name="offset">The offset in the buffer to start uploading from</param>
        /// <returns>The event representing the asynchronous upload operation</returns>
        virtual std::shared_ptr<IEvent> UploadAsync(IQueue& queue,
                                                    const void* src,
                                                    size_t bytes,
                                                    size_t offset = 0) = 0;

        /// <summary>
		/// Downloads data from the buffer to the specified destination asynchronously using the provided queue.
        /// </summary>
        /// <param name="queue">The queue to use for the copy operation</param>
        /// <param name="dst">The destination buffer to download data to</param>
        /// <param name="bytes">The number of bytes to download</param>
        /// <param name="offset">The offset in the buffer to start downloading from</param>
        /// <returns>The event representing the asynchronous download operation</returns>
        virtual std::shared_ptr<IEvent> DownloadAsync(IQueue& queue,
                                                      void* dst,
                                                      size_t bytes,
                                                      size_t offset = 0) = 0;

        /// <summary>
		/// Copies data from the specified buffer to this buffer using the provided queue.
        /// </summary>
        /// <param name="queue">The queue to use for the copy operation</param>
        /// <param name="buffer">The buffer to copy data from</param>
        /// <returns>True if the copy operation was successful, false otherwise</returns>
        virtual bool Copy(IQueue& queue,
                          IBuffer& buffer) = 0;
    };
}