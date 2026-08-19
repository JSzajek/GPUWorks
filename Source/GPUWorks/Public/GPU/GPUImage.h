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

    /// <summary>
	/// Retrieves the size in bytes of a pixel format.
    /// </summary>
    /// <param name="format">The pixel format</param>
    /// <returns>The size in bytes of the pixel format</returns>
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

    /// <summary>
	/// Image region structure that defines a sub-region of an image for operations such as upload, download, or fill.
    /// </summary>
    struct ImageRegion
    {
        uint32_t X = 0;
        uint32_t Y = 0;
        uint32_t Z = 0;

        uint32_t Width = 1;
        uint32_t Height = 1;
        uint32_t Depth = 1;
    };

    /// <summary>
	/// Image layout structure that defines the layout 
    /// of image data in memory, including row and slice pitch in bytes.
    /// </summary>
    struct ImageLayout
    {
        size_t RowPitchBytes = 0;
        size_t SlicePitchBytes = 0;
    };

    /// <summary>
	/// Interface for a GPU image resource, providing 
    /// methods for querying image properties and performing 
    /// operations such as upload, download, and fill.
    /// </summary>
    class IImage
    {
    public:
        /// <summary>
        /// Default destructor.
        /// </summary>
        virtual ~IImage() = default;

        /// <summary>
		/// Retrieves the backend type of the image resource.
        /// </summary>
        /// <returns>The backend type of the image resource</returns>
        virtual Backend GetBackend() const = 0;

        /// <summary>
		/// Retrieves the image description.
        /// </summary>
        /// <returns>The image description</returns>
        virtual const ImageDescription& GetDesc() const = 0;

        /// <summary>
		/// Retrieves the width of the image in pixels.
        /// </summary>
        /// <returns>The width</returns>
        virtual uint32_t GetWidth() const = 0;

        /// <summary>
		/// Retrieves the height of the image in pixels.
        /// </summary>
        /// <returns>The height</returns>
        virtual uint32_t GetHeight() const = 0;

        /// <summary>
		/// Retrieves the depth or number of layers of the image.
        /// </summary>
        /// <returns>The depth or layers</returns>
        virtual uint32_t GetDepthOrLayers() const = 0;

        /// <summary>
		/// Retrieves the number of channels of the image based on its pixel format.
        /// </summary>
        /// <returns>The number of channels</returns>
        virtual uint32_t GetChannels() const = 0;

        /// <summary>
		/// Gets the pixel format of the image.
        /// </summary>
        /// <returns>The pixel format</returns>
        virtual PixelFormat GetFormat() const = 0;

        /// <summary>
		/// Retrieves the size in bytes of the image data based on its dimensions and pixel format.
        /// </summary>
        /// <returns>The size in bytes</returns>
        virtual uint32_t GetBytesSize() const = 0;

        /// <summary>
		/// Retrieves the image type of the image resource.
        /// </summary>
        /// <returns>The image type</returns>
        virtual ImageType GetType() const = 0;

        /// <summary>
		/// Uploads data to the image resource from the specified source data, region, and layout.
        /// </summary>
        /// <param name="queue">The queue to use for the upload operation</param>
        /// <param name="srcData">Pointer to the source data</param>
        /// <param name="srcBytes">Size of the source data in bytes</param>
        /// <param name="region">The region of the image to upload</param>
        /// <param name="layout">The layout of the image data</param>
        /// <returns>True if the upload was successful, false otherwise</returns>
        virtual bool Upload(IQueue& queue,
                            const void* srcData,
                            size_t srcBytes,
                            const ImageRegion& region,
                            const ImageLayout& layout = {}) = 0;
		
        /// <summary>
		/// Downloads data from the image resource to the specified destination data, region, and layout.
        /// </summary>
        /// <param name="queue">The queue to use for the upload operation</param>
        /// <param name="dstData">Pointer to the destination data</param>
        /// <param name="dstBytes">Size of the destination data in bytes</param>
        /// <param name="region">The region of the image to upload</param>
        /// <param name="layout">The layout of the image data</param>
        /// <returns>True if the upload was successful, false otherwise</returns>
        virtual bool Download(IQueue& queue,
                              void* dstData,
                              size_t dstBytes,
                              const ImageRegion& region,
                              const ImageLayout& layout = {}) = 0;

        /// <summary>
		/// Fills a region of the image resource with a specified value.
        /// </summary>
        /// <param name="queue">The queue to use for the upload operation</param>
        /// <param name="value">The value to fill the region with</param>
        /// <param name="valueSize">The size of the value in bytes</param>
        /// <param name="region">The region of the image to fill</param>
        /// <returns>True if the upload was successful, false otherwise</returns>
        virtual bool Fill(IQueue& queue,
                          const void* value,
                          size_t valueSize,
                          const ImageRegion& region) = 0;

        /// <summary>
		/// Uploads data to the image resource asynchronously from the specified source data, region, and layout.
        /// </summary>
        /// <param name="queue">The queue to use for the upload operation</param>
        /// <param name="srcData">Pointer to the source data</param>
        /// <param name="srcBytes">Size of the source data in bytes</param>
        /// <param name="region">The region of the image to upload</param>
        /// <param name="layout">The layout of the image data</param>
        /// <returns>The event representing the asynchronous operation</returns>
        virtual std::shared_ptr<IEvent> UploadAsync(IQueue& queue,
                                                    const void* srcData,
                                                    size_t srcBytes,
                                                    const ImageRegion& region,
                                                    const ImageLayout& layout = {}) = 0;

        /// <summary>
		/// Downloads data from the image resource asynchronously to the specified destination data, region, and layout.
        /// </summary>
        /// <param name="queue">The queue to use for the upload operation</param>
        /// <param name="dstData">Pointer to the destination data</param>
        /// <param name="dstBytes">Size of the destination data in bytes</param>
        /// <param name="region">The region of the image to upload</param>
        /// <param name="layout">The layout of the image data</param>
        /// <returns>The event representing the asynchronous operation</returns>
        virtual std::shared_ptr<IEvent> DownloadAsync(IQueue& queue,
                                                      void* dstData,
                                                      size_t dstBytes,
                                                      const ImageRegion& region,
                                                      const ImageLayout& layout = {}) = 0;

        /// <summary>
		/// Fill a region of the image resource asynchronously with a specified value.
        /// </summary>
        /// <param name="queue">The queue to use for the upload operation</param>
        /// <param name="value">The value to fill the region with</param>
        /// <param name="valueSize">The size of the value in bytes</param>
        /// <param name="region">The region of the image to fill</param>
        /// <returns>The event representing the asynchronous operation</returns>
        virtual std::shared_ptr<IEvent> FillAsync(IQueue& queue,
                                                  const void* value,
                                                  size_t valueSize,
                                                  const ImageRegion& region) = 0;
    };
}