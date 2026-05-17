#include "Backends/OpenCL/OpenCLImage.h"
#include "Backends/OpenCL/OpenCLQueue.h"
#include "Backends/OpenCL/OpenCLContext.h"

namespace Gpu::OpenCL
{
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
        return Gpu::GetPixelFormatSize(mDescription.Format);
    }

    uint32_t Image::GetBytesSize() const
    {
        return mDescription.Width * mDescription.Height * mDescription.DepthOrLayers * Gpu::GetPixelFormatSize(mDescription.Format);
    }

    bool Image::Upload(IQueue& queueBase,
                       const void* srcData,
                       size_t srcBytes,
                       const ImageRegion& region,
                       const ImageLayout& layout)
    {
        OpenCL::Queue* queue = reinterpret_cast<OpenCL::Queue*>(&queueBase);
        if (!queue || !ImageObject || !srcData)
        {
            return false;
        }

        const size_t origin[3] = { region.X, region.Y, region.Z };
        const size_t clRegion[3] = { region.Width, region.Height, region.Depth };

        const cl_int err = clEnqueueWriteImage(queue->GetCLQueue(),
                                               ImageObject,
                                               CL_TRUE,
                                               origin,
                                               clRegion,
                                               layout.RowPitchBytes,
                                               layout.SlicePitchBytes,
                                               srcData,
                                               0,
                                               nullptr,
                                               nullptr);

        return err == CL_SUCCESS;
    }

    bool Image::Download(IQueue& queueBase,
                         void* dstData,
                         size_t dstBytes,
                         const ImageRegion& region,
                         const ImageLayout& layout)
    {
        OpenCL::Queue* queue = reinterpret_cast<OpenCL::Queue*>(&queueBase);
        if (!queue || !ImageObject || !dstData)
        {
            return false;
        }

        const size_t origin[3] = { region.X, region.Y, region.Z };
        const size_t clRegion[3] = { region.Width, region.Height, region.Depth };

        const cl_int err = clEnqueueReadImage(queue->GetCLQueue(),
                                              ImageObject,
                                              CL_TRUE,
                                              origin,
                                              clRegion,
                                              layout.RowPitchBytes,
                                              layout.SlicePitchBytes,
                                              dstData,
                                              0,
                                              nullptr,
                                              nullptr);

        return err == CL_SUCCESS;
    }

    std::shared_ptr<IEvent> Image::Fill(IQueue& queueBase,
                                        const void* value,
                                        size_t valueSize,
                                        const ImageRegion& region)
    {
        OpenCL::Queue* queue = reinterpret_cast<OpenCL::Queue*>(&queueBase);
        if (!queue || !ImageObject || !value)
        {
            return nullptr;
        }

    #if defined(CL_VERSION_1_2)
        const size_t origin[3] = { region.X, region.Y, region.Z };
        const size_t clRegion[3] = { region.Width, region.Height, region.Depth };

        cl_event ev = nullptr;
        const cl_int err = clEnqueueFillImage(queue->GetCLQueue(),
                                              ImageObject,
                                              value,
                                              origin,
                                              clRegion,
                                              0,
                                              nullptr,
                                              &ev);

        if (err != CL_SUCCESS)
        {
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
                out.image_channel_order = CL_RGBA;
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
}