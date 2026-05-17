#pragma once

#include "GPU/GPUTypes.h"
#include "GPU/GPUContext.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace Gpu
{
    class IQueue;
    class IEvent;

    static uint32_t GetPixelFormatSize(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::R8:
                return 1;
            case PixelFormat::RG8:
                return 2;
            case PixelFormat::RGBA8:
                return 4;
            case PixelFormat::R16F:
                return 2;
            case PixelFormat::RG16F:
                return 4;
            case PixelFormat::RGBA16F:
                return 8;
            case PixelFormat::R32F:
                return 4;
            case PixelFormat::RG32F:
                return 8;
            case PixelFormat::RGBA32F:
                return 16;
            case PixelFormat::R32U:
                return 4;
            case PixelFormat::RG32U:
                return 8;
            case PixelFormat::RGBA32U:
                return 16;
            case PixelFormat::R32S:
                return 4;
            default:
                return 0;
		}
    }

    struct ImageRegion
    {
        uint32_t X = 0;
        uint32_t Y = 0;
        uint32_t Z = 0;

        uint32_t Width = 1;
        uint32_t Height = 1;
        uint32_t Depth = 1;
    };

    struct ImageLayout
    {
        size_t RowPitchBytes = 0;
        size_t SlicePitchBytes = 0;
    };

    class IImage
    {
    public:
        virtual ~IImage() = default;

        virtual Backend GetBackend() const = 0;
        virtual const ImageDescription& GetDesc() const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetDepthOrLayers() const = 0;
        virtual uint32_t GetChannels() const = 0;
        virtual PixelFormat GetFormat() const = 0;
        virtual uint32_t GetBytesSize() const = 0;
        virtual ImageType GetType() const = 0;

        virtual bool Upload(IQueue& queue,
                            const void* srcData,
                            size_t srcBytes,
                            const ImageRegion& region,
                            const ImageLayout& layout = {}) = 0;

        virtual bool Download(IQueue& queue,
                              void* dstData,
                              size_t dstBytes,
                              const ImageRegion& region,
                              const ImageLayout& layout = {}) = 0;

        virtual std::shared_ptr<IEvent> Fill(IQueue& queue,
                                             const void* value,
                                             size_t valueSize,
                                             const ImageRegion& region) = 0;
    };
}