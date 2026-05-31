#include "Backends/OpenCL/OpenCLImage.h"
#include "Backends/OpenCL/OpenCLQueue.h"
#include "Backends/OpenCL/OpenCLContext.h"
#include "Backends/OpenCL/OpenCLEvent.h"

namespace Gpu::OpenCL
{
    namespace
    {
        struct AsyncImageUploadState
        {
            std::shared_ptr<Event> CompletionEvent;
            std::vector<uint8_t> StagingBytes;
        };

        static void CL_CALLBACK OnAsyncImageUploadComplete(cl_event EventHandle,
                                                           cl_int EventStatus,
                                                           void* UserData)
        {
            std::unique_ptr<AsyncImageUploadState> State(static_cast<AsyncImageUploadState*>(UserData));
            if (State && State->CompletionEvent)
            {
                State->CompletionEvent->IsComplete();
            }

            if (EventHandle)
            {
                clReleaseEvent(EventHandle);
            }
        }
    }

    static cl_mem_flags ToCLMemFlags(const ImageDescription& desc)
    {
        cl_mem_flags flags = 0;

        switch (desc.AccessMode)
        {
            case Access::ReadOnly:
            {
                flags |= CL_MEM_READ_ONLY;
                break;
            }
            case Access::WriteOnly:
            {
                flags |= CL_MEM_WRITE_ONLY;
                break;
            }
            case Access::ReadWrite:
            {
                flags |= CL_MEM_READ_WRITE;
                break;
            }
        }
        return flags;
    }

    Image::Image(const std::shared_ptr<IContext>& inContext,
                 const ImageDescription& inDesc,
                 cl_mem inImage)
        : ContextWeak(inContext),
        mDescription(inDesc),
        ImageObject(inImage)
    {
    }

    Image::~Image()
    {
        if (ImageObject)
        {
            clReleaseMemObject(ImageObject);
            ImageObject = nullptr;
        }
    }

    uint32_t Image::GetChannels() const
    {
        switch (mDescription.Format)
        {
            case PixelFormat::R8:
            case PixelFormat::R16F:
            case PixelFormat::R32F:
            case PixelFormat::R32U:
            case PixelFormat::R32S:
            {
                return 1;
            }
            case PixelFormat::RG8:
            case PixelFormat::RG16F:
            case PixelFormat::RG32F:
            case PixelFormat::RG32U:
            {
                return 2;
            }
            case PixelFormat::RGBA8:
            case PixelFormat::RGBA16F:
            case PixelFormat::RGBA32F:
            case PixelFormat::RGBA32U:
            {
                return 4;
            }
            default:
            {
                return 0;
            }
        }
    }

    uint32_t Image::GetBytesSize() const
    {
        return mDescription.Width * mDescription.Height * mDescription.DepthOrLayers * Gpu::GetPixelFormatSize(mDescription.Format);
    }

    bool Image::Upload(IQueue& queue,
                       const void* srcData,
                       size_t srcBytes,
                       const ImageRegion& region,
                       const ImageLayout& layout)
    {
        std::shared_ptr<IEvent> _event = UploadAsync(queue, srcData, srcBytes, region, layout);
        if (!_event)
        {
            return false;
        }

        _event->Wait();
        return _event->IsComplete();
    }

    bool Image::Download(IQueue& queue,
                         void* dstData,
                         size_t dstBytes,
                         const ImageRegion& region,
                         const ImageLayout& layout)
    {
        std::shared_ptr<IEvent> _event = DownloadAsync(queue, dstData, dstBytes, region, layout);
        if (!_event)
        {
            return false;
        }

        _event->Wait();
        return _event->IsComplete();
    }

    bool Image::Fill(IQueue& queue,
                     const void* value,
                     size_t valueSize,
                     const ImageRegion& region)
    {
        std::shared_ptr<IEvent> _event = FillAsync(queue, value, valueSize, region);
        if (!_event)
        {
            return false;
        }

        _event->Wait();
        return _event->IsComplete();
    }

    std::shared_ptr<Gpu::IEvent> Image::UploadAsync(IQueue& queue,
                                                    const void* srcData,
                                                    size_t srcBytes,
                                                    const ImageRegion& region,
                                                    const ImageLayout& layout)
    {
        OpenCL::Queue* _queue = reinterpret_cast<OpenCL::Queue*>(&queue);
        if (!_queue || !ImageObject || !srcData)
        {
            Gpu::OpenCL::LogCLError("Failed to Upload Async", CL_INVALID_VALUE);
            return nullptr;
        }

        const size_t RequiredBytes = GetRequiredBytes(region, layout);
        if (RequiredBytes == 0 || srcBytes < RequiredBytes)
        {
            Gpu::OpenCL::LogCLError("Failed to Upload Async", CL_INVALID_VALUE);
            return nullptr;
        }

        const size_t origin[3] =
        {
            static_cast<size_t>(region.X),
            static_cast<size_t>(region.Y),
            static_cast<size_t>(region.Z)
        };

        const size_t clRegion[3] =
        {
            static_cast<size_t>(region.Width),
            static_cast<size_t>(region.Height),
            static_cast<size_t>(region.Depth)
        };

        std::shared_ptr<Event> Completion = std::make_shared<Event>();

        auto* State = new AsyncImageUploadState();
        State->CompletionEvent = Completion;
        State->StagingBytes.resize(RequiredBytes);
        std::memcpy(State->StagingBytes.data(), srcData, RequiredBytes);

        cl_event WriteEvent = nullptr;
        cl_int err = clEnqueueWriteImage(_queue->GetCLQueue(),
                                         ImageObject,
                                         CL_FALSE,
                                         origin,
                                         clRegion,
                                         layout.RowPitchBytes,
                                         layout.SlicePitchBytes,
                                         State->StagingBytes.data(),
                                         0,
                                         nullptr,
                                         &WriteEvent);

        if (err != CL_SUCCESS || !WriteEvent)
        {
            Gpu::OpenCL::LogCLError("Failed to Enqueue Write Image", err);
            delete State;
            return nullptr;
        }

        err = clSetEventCallback(WriteEvent,
                                 CL_COMPLETE,
                                 &OnAsyncImageUploadComplete,
                                 State);

        if (err != CL_SUCCESS)
        {
            Gpu::OpenCL::LogCLError("Failed to Set Event Callback", err);
            clReleaseEvent(WriteEvent);
            delete State;
            return nullptr;
        }

        return Completion;
    }

    std::shared_ptr<Gpu::IEvent> Image::DownloadAsync(IQueue& queue,
                                                      void* dstData,
                                                      size_t dstBytes,
                                                      const ImageRegion& region,
                                                      const ImageLayout& layout)
    {
        OpenCL::Queue* _queue = reinterpret_cast<OpenCL::Queue*>(&queue);
        if (!_queue || !ImageObject || !dstData)
        {
            Gpu::OpenCL::LogCLError("Failed to Download Async", CL_INVALID_VALUE);
            return nullptr;
        }

        const size_t requiredBytes = GetRequiredBytes(region, layout);
        if (requiredBytes == 0 || dstBytes < requiredBytes)
        {
            Gpu::OpenCL::LogCLError("Failed to Download Async", CL_INVALID_VALUE);
            return nullptr;
        }

        const size_t origin[3] =
        {
            static_cast<size_t>(region.X),
            static_cast<size_t>(region.Y),
            static_cast<size_t>(region.Z)
        };

        const size_t clRegion[3] =
        {
            static_cast<size_t>(region.Width),
            static_cast<size_t>(region.Height),
            static_cast<size_t>(region.Depth)
        };

        cl_event readEvent = nullptr;
        const cl_int err = clEnqueueReadImage(_queue->GetCLQueue(),
                                              ImageObject,
                                              CL_FALSE,
                                              origin,
                                              clRegion,
                                              layout.RowPitchBytes,
                                              layout.SlicePitchBytes,
                                              dstData,
                                              0,
                                              nullptr,
                                              &readEvent);

        if (err != CL_SUCCESS || !readEvent)
        {
            Gpu::OpenCL::LogCLError("Failed to Enqueue Read Image", err);
            return nullptr;
        }

        return std::static_pointer_cast<IEvent>(std::make_shared<Event>(readEvent));
    }

    std::shared_ptr<Gpu::IEvent> Image::FillAsync(IQueue& queue,
                                                  const void* value,
                                                  size_t valueSize,
                                                  const ImageRegion& region)
    {
        OpenCL::Queue* _queue = reinterpret_cast<OpenCL::Queue*>(&queue);
        if (!_queue || !ImageObject || !value)
        {
            Gpu::OpenCL::LogCLError("Failed to Fill Async", CL_INVALID_VALUE);
            return nullptr;
        }

    #if defined(CL_VERSION_1_2)
        const size_t origin[3] = { region.X, region.Y, region.Z };
        const size_t clRegion[3] = { region.Width, region.Height, region.Depth };

        cl_event ev = nullptr;
        const cl_int err = clEnqueueFillImage(_queue->GetCLQueue(),
                                              ImageObject,
                                              value,
                                              origin,
                                              clRegion,
                                              0,
                                              nullptr,
                                              &ev);

        if (err != CL_SUCCESS)
        {
            Gpu::OpenCL::LogCLError("Failed to Enqueue Fill Image", err);
            return nullptr;
        }
        return static_pointer_cast<IEvent>(std::make_shared<Event>(ev));
    #else
        return nullptr;
    #endif
    }

    cl_image_format Image::ToCLImageFormat(PixelFormat format)
    {
        cl_image_format out = {};

        switch (format)
        {
            case PixelFormat::R8:
            {
                out.image_channel_order = CL_R;
                out.image_channel_data_type = CL_UNORM_INT8;
                break;
            }
            case PixelFormat::RG8:
            {
                out.image_channel_order = CL_RG;
                out.image_channel_data_type = CL_UNORM_INT8;
                break;
            }
            case PixelFormat::RGBA8:
            {
                out.image_channel_order = CL_RGBA;
                out.image_channel_data_type = CL_UNORM_INT8;
                break;
            }
            case PixelFormat::R16F:
            {
                out.image_channel_order = CL_R;
                out.image_channel_data_type = CL_HALF_FLOAT;
                break;

            }
            case PixelFormat::RG16F:
            {
                out.image_channel_order = CL_RG;
                out.image_channel_data_type = CL_HALF_FLOAT;
                break;

            }
            case PixelFormat::RGBA16F:
            {
                out.image_channel_order = CL_RGBA;
                out.image_channel_data_type = CL_HALF_FLOAT;
                break;
            }
            case PixelFormat::R32F:
            {
                out.image_channel_order = CL_R;
                out.image_channel_data_type = CL_FLOAT;
                break;

            }
            case PixelFormat::RG32F:
            {
                out.image_channel_order = CL_RG;
                out.image_channel_data_type = CL_FLOAT;
                break;

            }
            case PixelFormat::RGBA32F:
            {
                out.image_channel_order = CL_RGBA;
                out.image_channel_data_type = CL_FLOAT;
                break;
            }
            case PixelFormat::R32U:
            {
                out.image_channel_order = CL_R;
                out.image_channel_data_type = CL_UNSIGNED_INT32;
                break;

            }
            case PixelFormat::RG32U:
            {
                out.image_channel_order = CL_RG;
                out.image_channel_data_type = CL_UNSIGNED_INT32;
                break;

            }
            case PixelFormat::RGBA32U:
            {
                out.image_channel_order = CL_RGBA;
                out.image_channel_data_type = CL_UNSIGNED_INT32;
                break;
            }
            case PixelFormat::R32S:
            {
                out.image_channel_order = CL_R;
                out.image_channel_data_type = CL_SIGNED_INT32;
                break;
            }
            default:
                break;
        }
        return out;
    }

    cl_image_desc Image::ToCLImageDesc(const ImageDescription& desc)
    {
        cl_image_desc out = {};
        out.image_width = desc.Width;
        out.image_height = desc.Height;
        out.image_depth = (desc.Type == ImageType::Tex3D) ? desc.DepthOrLayers : 0;
        out.image_array_size = (desc.Type == ImageType::Tex2DArray) ? desc.DepthOrLayers : 0;

        switch (desc.Type)
        {
            case ImageType::Tex2D:
            {
                out.image_type = CL_MEM_OBJECT_IMAGE2D;
                break;
            }
            case ImageType::Tex2DArray:
            {
                out.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
                break;
            }
            case ImageType::Tex3D:
            {
                out.image_type = CL_MEM_OBJECT_IMAGE3D;
                break;
            }
        }
        return out;
    }

	size_t Image::GetRequiredBytes(const ImageRegion& region,
                                   const ImageLayout& layout) const
	{
        const size_t BytesPerPixel = Gpu::GetPixelFormatSize(mDescription.Format);
        if (BytesPerPixel == 0)
        {
            return 0;
        }

        const size_t RowPitch = layout.RowPitchBytes != 0 ? layout.RowPitchBytes :
                                                            static_cast<size_t>(region.Width) * BytesPerPixel;

        const size_t SlicePitch = layout.SlicePitchBytes != 0 ? layout.SlicePitchBytes :
                                                                RowPitch * static_cast<size_t>(region.Height);

        return SlicePitch * static_cast<size_t>(region.Depth);
	}
}