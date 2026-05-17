#include "Backends/OpenCL/OpenCLBuffer.h"
#include "Backends/OpenCL/OpenCLQueue.h"
#include "Backends/OpenCL/OpenCLDevice.h"
#include "Backends/OpenCL/OpenCLContext.h"
#include "Backends/OpenCL/OpenCLKernel.h"
#include "Backends/OpenCL/OpenCLEvent.h"

namespace Gpu::OpenCL
{
struct AsyncMappedTransferState
    {
        std::shared_ptr<Event> mpCompletionEvent;
        std::weak_ptr<Queue> mpQueue;

        cl_mem mpMemObject = nullptr;
        void* mpSVMPtr = nullptr;
        void* mpMappedPtr = nullptr;

        std::vector<uint8_t> mUploadStaging;

        void* mpDownloadDest = nullptr;

        size_t mBytes = 0;
        size_t mOffset = 0;

        bool mIsUpload = false;
        bool mIsSVM = false;
    };

    static void CL_CALLBACK OnMappedTransferReady(cl_event _event, 
                                                  cl_int _eventCommandStatus, 
                                                  void* userData)
    {
        std::unique_ptr<AsyncMappedTransferState> State(static_cast<AsyncMappedTransferState*>(userData));
        if (!State)
        {
            return;
        }

        std::shared_ptr<Queue> QueuePtr = State->mpQueue.lock();
        if (!QueuePtr || _eventCommandStatus != CL_COMPLETE)
        {
            State->mpCompletionEvent->IsComplete();
            return;
        }

        cl_command_queue CLQueue = QueuePtr->GetCLQueue();
        if (State->mIsUpload)
        {
            const uint8_t* Src = State->mUploadStaging.data();
            if (State->mIsSVM)
            {
                std::memcpy(static_cast<uint8_t*>(State->mpSVMPtr) + State->mOffset,
                            Src, 
                            State->mBytes);
            }
            else
            {
                std::memcpy(static_cast<uint8_t*>(State->mpMappedPtr) + State->mOffset,
                            Src,
                            State->mBytes);
            }
        }
        else
        {
            if (State->mpDownloadDest)
            {
                if (State->mIsSVM)
                {
                    std::memcpy(State->mpDownloadDest,
                                static_cast<uint8_t*>(State->mpSVMPtr) + State->mOffset,
                                State->mBytes);
                }
                else
                {
                    std::memcpy(State->mpDownloadDest,
                                static_cast<uint8_t*>(State->mpMappedPtr) + State->mOffset,
                                State->mBytes);
                }
            }
        }

        cl_event UnmapEvent = nullptr;
        if (State->mIsSVM)
        {
            clEnqueueSVMUnmap(CLQueue,
                              State->mpSVMPtr,
                              0,
                              nullptr,
                              &UnmapEvent);
        }
        else
        {
            clEnqueueUnmapMemObject(CLQueue,
                                    State->mpMemObject,
                                    State->mpMappedPtr,
                                    0,
                                    nullptr,
                                    &UnmapEvent);
        }

        if (!UnmapEvent)
        {
            State->mpCompletionEvent->IsComplete();
            return;
        }

        std::shared_ptr<Event>* CompletionCopy = new std::shared_ptr<Event>(State->mpCompletionEvent);
        clSetEventCallback(UnmapEvent, 
                           CL_COMPLETE, 
                           [](cl_event FinishedEvent,
                              cl_int,
                              void* UserData2)
        {
            std::unique_ptr<std::shared_ptr<Event>> Completion(static_cast<std::shared_ptr<Event>*>(UserData2));
            if (Completion && *Completion)
            {
                (*Completion)->IsComplete();
            }

            // Release the unmap event
            clReleaseEvent(FinishedEvent);

        }, new std::shared_ptr<Event>(State->mpCompletionEvent));
    }

    Buffer::Buffer(const std::shared_ptr<Context>& inContext,
                   const BufferDescription& desc,
                   cl_mem inMem)
        : mpContext(inContext),
        mDescription(desc),
        mpMemObject(inMem)
    {
        Initialize();
    }

    Buffer::~Buffer()
    {
        if (mpMemObject)
        {
            clReleaseMemObject(mpMemObject);
            mpMemObject = nullptr;
        }
    }

    bool Buffer::AttachToKernel(IKernel& kernelBase,
                                uint32_t argIndex)
    {
        OpenCL::Kernel* kernel = reinterpret_cast<OpenCL::Kernel*>(&kernelBase);

        cl_int err = CL_SUCCESS;
        if (mResolvedSyncMode == BufferSyncMode::ZeroCopy)
        {
            err = clSetKernelArgSVMPointer(kernel->GetCLKernel(), argIndex, mpSVM);
        }
        else
        {
            err = clSetKernelArg(kernel->GetCLKernel(), argIndex, sizeof(cl_mem), &mpMemObject);
        }

        if (err != CL_SUCCESS)
        {
            kernel->mIsValid = false;

            UE_LOG(LogTemp, Warning, TEXT("Failed To Attach Buffer To Kernel: %s"), *FString(GetErrorString(err).c_str()));
            return false;
        }
        return true;
    }

    bool Buffer::Upload(IQueue& queueBase,
                        const void* src,
                        size_t bytes,
                        size_t offset)
    {
        OpenCL::Queue* queue = reinterpret_cast<OpenCL::Queue*>(&queueBase);
        if (!queue || !src || offset + bytes > mDescription.SizeBytes)
        {
            return false;
        }

        switch (mResolvedSyncMode)
        {
            case BufferSyncMode::CopyOnce:
            {
                return false;
            }
            case BufferSyncMode::Stream:
            {
                cl_int err = CL_SUCCESS;
                void* hostPtr = clEnqueueMapBuffer(queue->GetCLQueue(),
                                                   mpMemObject,
                                                   CL_TRUE,
                                                   CL_MAP_WRITE,
                                                   0,
                                                   bytes,
                                                   0,
                                                   nullptr,
                                                   nullptr,
                                                   &err);

                if (!hostPtr || err != CL_SUCCESS)
                {
                    return false;
                }

                std::memcpy(static_cast<uint8_t*>(hostPtr) + offset, src, bytes);

                return clEnqueueUnmapMemObject(queue->GetCLQueue(),
                                               mpMemObject,
                                               hostPtr,
                                               0,
                                               nullptr,
                                               nullptr) == CL_SUCCESS;
            }
            case BufferSyncMode::ZeroCopy:
            {
                if (!mpSVM)
                {
                    return false;
                }

                if (clEnqueueSVMMap(queue->GetCLQueue(),
                                    CL_TRUE,
                                    CL_MAP_WRITE,
                                    mpSVM,
                                    bytes,
                                    0,
                                    nullptr,
                                    nullptr) != CL_SUCCESS)
                {
                    return false;
                }

                std::memcpy(static_cast<uint8_t*>(mpSVM) + offset, src, bytes);

                return clEnqueueSVMUnmap(queue->GetCLQueue(),
                                         mpSVM,
                                         0,
                                         nullptr,
                                         nullptr) == CL_SUCCESS;
            }
            default:
            {
                return false;
            }
        }
    }

    bool Buffer::Download(IQueue& queueBase,
                          void* dst,
                          size_t bytes,
                          size_t offset)
    {
        OpenCL::Queue* queue = reinterpret_cast<OpenCL::Queue*>(&queueBase);
        if (!queue || !mpMemObject || !dst || offset + bytes > mDescription.SizeBytes)
        {
            return false;
        }

        switch (mResolvedSyncMode)
        {
            case BufferSyncMode::CopyOnce:
            {
                return clEnqueueReadBuffer(queue->GetCLQueue(),
                                           mpMemObject,
                                           CL_TRUE,
                                           offset,
                                           bytes,
                                           dst,
                                           0,
                                           nullptr,
                                           nullptr) == CL_SUCCESS;
            }
            case BufferSyncMode::Stream:
            {
                cl_int err = CL_SUCCESS;
                void* hostPtr = clEnqueueMapBuffer(queue->GetCLQueue(),
                                                   mpMemObject,
                                                   CL_TRUE,
                                                   CL_MAP_READ,
                                                   0,
                                                   bytes,
                                                   0,
                                                   nullptr,
                                                   nullptr,
                                                   &err);

                if (!hostPtr || err != CL_SUCCESS)
                {
                    return false;
                }

                std::memcpy(dst, static_cast<uint8_t*>(hostPtr) + offset, bytes);

                return clEnqueueUnmapMemObject(queue->GetCLQueue(),
                                               mpMemObject,
                                               hostPtr,
                                               0,
                                               nullptr,
                                               nullptr) == CL_SUCCESS;
            }
            case BufferSyncMode::ZeroCopy:
            {
                if (!mpSVM)
                {
                    return false;
                }

                if (clEnqueueSVMMap(queue->GetCLQueue(),
                                    CL_TRUE,
                                    CL_MAP_READ,
                                    mpSVM,
                                    bytes,
                                    0,
                                    nullptr,
                                    nullptr) != CL_SUCCESS)
                {
                    return false;
                }

                std::memcpy(dst, static_cast<uint8_t*>(mpSVM) + offset, bytes);

                return clEnqueueSVMUnmap(queue->GetCLQueue(),
                                         mpSVM,
                                         0,
                                         nullptr,
                                         nullptr) == CL_SUCCESS;
            }
            default:
            {
                return false;
            }
        }
    }

	std::shared_ptr<IEvent> Buffer::UploadAsync(IQueue& queueBase,
                                                const void* src,
                                                size_t bytes,
                                                size_t offset)
	{
        OpenCL::Queue* queue = reinterpret_cast<OpenCL::Queue*>(&queueBase);
        if (!queue || !src || offset + bytes > mDescription.SizeBytes)
        {
            return nullptr;
        }

        cl_event ev = nullptr;
        switch (mResolvedSyncMode)
        {
            case BufferSyncMode::CopyOnce:
            {
                // Matches old behavior: COPY_ONCE buffers are immutable after creation.
                return nullptr;
            }
            case BufferSyncMode::Stream:
            {
                cl_int Err = CL_SUCCESS;
                void* HostPtr = clEnqueueMapBuffer(queue->GetCLQueue(),
                                                   mpMemObject,
                                                   CL_FALSE,
                                                   CL_MAP_WRITE,
                                                   0,
                                                   bytes + offset,
                                                   0,
                                                   nullptr,
                                                   &ev,
                                                   &Err);

                if (Err != CL_SUCCESS || !HostPtr || !ev)
                {
                    if (ev)
                    {
                        clReleaseEvent(ev);
                    }
                    return nullptr;
                }

                auto Completion = std::make_shared<Event>();
                auto* State = new AsyncMappedTransferState();
                State->mpCompletionEvent = Completion;
                State->mpQueue = queue->shared_from_this();
                State->mpMemObject = mpMemObject;
                State->mpMappedPtr = HostPtr;
                State->mBytes = bytes;
                State->mOffset = offset;
                State->mIsUpload = true;
                State->mIsSVM = false;

                State->mUploadStaging.resize(bytes);
                std::memcpy(State->mUploadStaging.data(), src, bytes);

                clSetEventCallback(ev,
                                   CL_COMPLETE,
                                   &OnMappedTransferReady,
                                   State);

                return Completion;
            }

            case BufferSyncMode::ZeroCopy:
            {
                if (!mpSVM)
                {
                    return nullptr;
                }

                const cl_int Err = clEnqueueSVMMap(queue->GetCLQueue(),
                                                   CL_FALSE,
                                                   CL_MAP_WRITE,
                                                   mpSVM,
                                                   bytes + offset,
                                                   0,
                                                   nullptr,
                                                   &ev);

                if (Err != CL_SUCCESS || !ev)
                {
                    if (ev)
                    {
                        clReleaseEvent(ev);
                    }
                    return nullptr;
                }

                auto Completion = std::make_shared<Event>();
                auto* State = new AsyncMappedTransferState();
                State->mpCompletionEvent = Completion;
                State->mpQueue = queue->shared_from_this();
                State->mpSVMPtr = mpSVM;
                State->mBytes = bytes;
                State->mOffset = offset;
                State->mIsUpload = true;
                State->mIsSVM = true;

                State->mUploadStaging.resize(bytes);
                std::memcpy(State->mUploadStaging.data(), src, bytes);

                clSetEventCallback(ev,
                                   CL_COMPLETE,
                                   &OnMappedTransferReady,
                                   State);

                return Completion;
            }
            default:
            {

                return nullptr;
            }
        }
	}

	std::shared_ptr<IEvent> Buffer::DownloadAsync(IQueue& queueBase,
                                                  void* dst,
                                                  size_t bytes,
                                                  size_t offset)
	{
        OpenCL::Queue* queue = reinterpret_cast<OpenCL::Queue*>(&queueBase);
        if (!queue || !dst || offset + bytes > mDescription.SizeBytes)
        {
            return nullptr;
        }

        cl_event ev = nullptr;
        std::shared_ptr<Context> context = mpContext.lock();
        switch (mResolvedSyncMode)
        {
            case BufferSyncMode::CopyOnce:
            {
                const cl_int err = clEnqueueReadBuffer(queue->GetCLQueue(),
                                                       mpMemObject,
                                                       CL_FALSE,
                                                       offset,
                                                       bytes,
                                                       dst,
                                                       0,
                                                       nullptr,
                                                       &ev);

                if (err != CL_SUCCESS)
                {
                    return nullptr;
                }
                return static_pointer_cast<IEvent>(std::make_shared<Event>(ev));
            }
            case BufferSyncMode::Stream:
            {
                cl_int Err = CL_SUCCESS;
                void* HostPtr = clEnqueueMapBuffer(queue->GetCLQueue(),
                                                   mpMemObject,
                                                   CL_FALSE,
                                                   CL_MAP_READ,
                                                   0,
                                                   bytes + offset,
                                                   0,
                                                   nullptr,
                                                   &ev,
                                                   &Err);

                if (Err != CL_SUCCESS || !HostPtr || !ev)
                {
                    if (ev)
                    {
                        clReleaseEvent(ev);
                    }
                    return nullptr;
                }

                auto Completion = std::make_shared<Event>(ev);
                auto* State = new AsyncMappedTransferState();
                State->mpCompletionEvent = Completion;
				State->mpQueue = queue->shared_from_this();
                State->mpMemObject = mpMemObject;
                State->mpMappedPtr = HostPtr;
                State->mpDownloadDest = dst;
                State->mBytes = bytes;
                State->mOffset = offset;
                State->mIsUpload = false;
                State->mIsSVM = false;

                clSetEventCallback(ev,
                                   CL_COMPLETE,
                                   &OnMappedTransferReady,
                                   State);

                return Completion;
            }
            case BufferSyncMode::ZeroCopy:
            {
                if (!mpSVM)
                {
                    return nullptr;
                }

                const cl_int Err = clEnqueueSVMMap(queue->GetCLQueue(),
                                                   CL_FALSE,
                                                   CL_MAP_READ,
                                                   mpSVM,
                                                   bytes + offset,
                                                   0,
                                                   nullptr,
                                                   &ev);

                if (Err != CL_SUCCESS || !ev)
                {
                    if (ev)
                    {
                        clReleaseEvent(ev);
                    }

                    return nullptr;
                }

                auto Completion = std::make_shared<Event>(ev);

                auto* State = new AsyncMappedTransferState();
                State->mpCompletionEvent = Completion;
                State->mpQueue = queue->shared_from_this();
                State->mpSVMPtr = mpSVM;
                State->mpDownloadDest = dst;
                State->mBytes = bytes;
                State->mOffset = offset;
                State->mIsUpload = false;
                State->mIsSVM = true;

                clSetEventCallback(ev,
                                   CL_COMPLETE,
                                   &OnMappedTransferReady,
                                   State);

                return Completion;
            }
            default:
            {
                return nullptr;
            }
        }
	}

	void Buffer::Initialize()
	{
        bool usedFallback = false;
        mResolvedSyncMode = ResolveSyncMode(usedFallback);

        cl_mem_flags flags = ToCLMemFlags(mDescription.AccessMode);

		std::shared_ptr<Context> context = mpContext.lock();

        switch (mResolvedSyncMode)
        {
            case BufferSyncMode::CopyOnce:
            {
                flags |= CL_MEM_COPY_HOST_PTR;
                mpMemObject = CreateBuffer(context->GetCLContext(),
                                           flags,
                                           mDescription.InitialData,
                                           mDescription.SizeBytes);

                mResolvedMemoryModel = ResolvedMemoryModel::CLBuffer;
                break;
            }
            case BufferSyncMode::Stream:
            {
                if (mDescription.InitialData)
                {
                    flags |= CL_MEM_COPY_HOST_PTR;
                }

                mpMemObject = CreateBuffer(context->GetCLContext(),
                                           flags,
                                           mDescription.InitialData,
                                           mDescription.SizeBytes);

                mResolvedMemoryModel = ResolvedMemoryModel::CLBuffer;
                break;
            }
            case BufferSyncMode::ZeroCopy:
            {
                mpSVM = clSVMAlloc(context->GetCLContext(),
                                   CL_MEM_READ_WRITE,
                                   mDescription.SizeBytes,
                                   0);

                if (mpSVM && mDescription.InitialData)
                {
                    std::shared_ptr<Gpu::IQueue> initQueue = context->CreateQueue();
                    Upload(*initQueue,
                           mDescription.InitialData,
                           mDescription.SizeBytes,
                           0);
                }

                mResolvedMemoryModel = ResolvedMemoryModel::SVM;
                break;
            }
            default:
            {
                break;
            }
        }
	}

    cl_mem Buffer::CreateBuffer(cl_context context,
                                cl_mem_flags flags,
                                const void* dataPtr,
                                size_t dataSize)
    {
        cl_int err = -1;
		cl_mem buffer = clCreateBuffer(context,
									   flags,
									   dataSize,
									   (void*)dataPtr,
									   &err);

		if (err < 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Couldn't Create Buffer: %d"), err);
			return nullptr;
		}
		return buffer;
    }

    BufferSyncMode Buffer::ResolveSyncMode(bool& bOutUsingFallback)
	{
		std::shared_ptr<Context> context = mpContext.lock();
        if (!context)
        {
            return BufferSyncMode::Auto;
        }

		std::shared_ptr<IDevice> device = context->GetDevice();

        bOutUsingFallback = false;

        BufferSyncMode requested = mDescription.SyncMode;
        if (requested == BufferSyncMode::Auto)
        {
            switch (mDescription.Usage)
            {
                case MemoryUsage::Shared:
                {
                    requested = BufferSyncMode::ZeroCopy;
                    break;
                }
                case MemoryUsage::Upload:
                case MemoryUsage::Readback:
                {
                    requested = BufferSyncMode::Stream;
                    break;
                }
                case MemoryUsage::Default:
                default:
                {
                    requested = mDescription.InitialData ? BufferSyncMode::CopyOnce : BufferSyncMode::Stream;
                    break;
                }
            }
        }

        if (requested == BufferSyncMode::ZeroCopy)
        {
            const bool bSupportsSVM = device && device->GetCapabilities().mSVMSupport != SVMSupport::None;
            if (!bSupportsSVM)
            {
                if (!mDescription.AllowBackendFallback)
                {
                    return BufferSyncMode::ZeroCopy;
                }

                bOutUsingFallback = true;
                return BufferSyncMode::Stream;
            }
        }
        return requested;
	}
}