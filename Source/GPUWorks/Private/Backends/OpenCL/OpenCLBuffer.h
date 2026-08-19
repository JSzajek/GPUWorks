#pragma once

#include "GPU/GPUContext.h"
#include "GPU/GPUBuffer.h"
#include "GPU/GPUNativeHandles.h"

#include "Backends/OpenCL/OpenCLCommon.h"

namespace Gpu::OpenCL
{
    class Context;
    class Device;
    class Queue;

    class Buffer final : public IBuffer,
                         public INativeHandleProvider
    {
    public:
        Buffer(const std::shared_ptr<Context>& inContext,
               const BufferDescription& desc,
               cl_mem inMem);

        virtual ~Buffer() override;

        virtual Backend GetBackend() const override { return Backend::OpenCL; }
        virtual size_t GetSize() const override { return mDescription.mSizeBytes; }
        virtual const BufferDescription& GetDescription() const override { return mDescription; }
        cl_mem GetCLMem() const { return mpMemObject; }

        virtual bool CanUploadAfterCreate() const override 
        {
            return mResolvedSyncMode != BufferSyncMode::CopyOnce;
        }

        virtual BufferSyncMode GetResolvedSyncMode() const override
        {
            return mResolvedSyncMode;
        }

        virtual bool AttachToKernel(IKernel& kernelBase,
                                    uint32_t argIndex) override;

        virtual bool Upload(IQueue& queueBase,
                            const void* src,
                            size_t bytes,
                            size_t offset = 0) override;

        virtual bool Download(IQueue& queueBase,
                              void* dst,
                              size_t bytes,
                              size_t offset = 0) override;

        virtual NativeHandle GetNativeHandle() const override
        {
            return { Backend::OpenCL, reinterpret_cast<void*>(mpMemObject) };
        }

        virtual std::shared_ptr<IEvent> UploadAsync(IQueue& queueBase,
                                                    const void* src,
                                                    size_t bytes,
                                                    size_t offset = 0) override;

        virtual std::shared_ptr<IEvent> DownloadAsync(IQueue& queueBase,
                                                      void* dst,
                                                      size_t bytes,
                                                      size_t offset = 0) override;

        virtual bool Copy(IQueue& queue,
                          IBuffer& buffer) override;
    private:
        enum class ResolvedMemoryModel : uint8_t
        {
            CLBuffer,
            SVM
        };
    private:
        void Initialize();

        cl_mem  CreateBuffer(cl_context context,
                             cl_mem_flags flags,
                             const void* dataPtr,
                             size_t dataSize);

        BufferSyncMode ResolveSyncMode(bool& bOutUsingFallback);
    private:
        std::weak_ptr<Context> mpContext;

        BufferDescription mDescription;
		BufferSyncMode mResolvedSyncMode = BufferSyncMode::Auto;
		ResolvedMemoryModel mResolvedMemoryModel = ResolvedMemoryModel::CLBuffer;

        cl_mem mpMemObject = nullptr;
        void* mpSVM = nullptr;
    };
}