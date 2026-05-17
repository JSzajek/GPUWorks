#pragma once

#include "GPU/GPUTypes.h"
#include "GPU/GPUContext.h"

namespace Gpu
{
    class IQueue;
    class IKernel;
    class IEvent;

    class IBuffer
    {
    public:
        virtual ~IBuffer() = default;

        virtual Backend GetBackend() const = 0;
        virtual size_t GetSize() const = 0;
        virtual const BufferDescription& GetDescription() const = 0;

        virtual bool CanUploadAfterCreate() const = 0;

        // The actual backend-selected mode after fallback / capability resolution
        virtual BufferSyncMode GetResolvedSyncMode() const = 0;

        virtual bool AttachToKernel(IKernel& kernel,
                                    uint32_t argIndex) = 0;

        virtual bool Upload(IQueue& queue,
                            const void* src,
                            size_t bytes,
                            size_t offset = 0) = 0;

        virtual bool Download(IQueue& queue,
                              void* dst,
                              size_t bytes,
                              size_t offset = 0) = 0;

        virtual std::shared_ptr<IEvent> UploadAsync(IQueue& queue,
                                                    const void* src,
                                                    size_t bytes,
                                                    size_t offset = 0) = 0;

        virtual std::shared_ptr<IEvent> DownloadAsync(IQueue& queue,
                                                      void* dst,
                                                      size_t bytes,
                                                      size_t offset = 0) = 0;
    };
}