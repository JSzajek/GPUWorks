#pragma once

#include "GPU/GPUImage.h"
#include "GPU/GPUNativeHandles.h"

#include "OpenCLLib.h"

#include <memory>

namespace Gpu
{
    class IContext;
    class IQueue;

    namespace OpenCL
    {
        class Image final : public IImage,
                            public INativeHandleProvider
        {
        public:
            Image(const std::shared_ptr<IContext>& inContext,
                  const ImageDescription& inDesc,
                  cl_mem inImage);

            virtual ~Image() override;

            virtual Backend GetBackend() const override { return Backend::OpenCL; }
            virtual const ImageDescription& GetDesc() const override { return mDescription; }

            virtual uint32_t GetWidth() const override { return mDescription.Width; }
            virtual uint32_t GetHeight() const override { return mDescription.Height; }
            virtual uint32_t GetDepthOrLayers() const override { return mDescription.DepthOrLayers; }
            virtual uint32_t GetChannels() const override;
            virtual PixelFormat GetFormat() const override { return mDescription.Format; }
            virtual uint32_t GetBytesSize() const override;
            virtual ImageType GetType() const override { return mDescription.Type; }

            virtual bool Upload(IQueue& queue,
                                const void* srcData,
                                size_t srcBytes,
                                const ImageRegion& region,
                                const ImageLayout& layout = {}) override;

            virtual bool Download(IQueue& queue,
                                  void* dstData,
                                  size_t dstBytes,
                                  const ImageRegion& region,
                                  const ImageLayout& layout = {}) override;

            virtual bool Fill(IQueue& queue,
                              const void* value,
                              size_t valueSize,
                              const ImageRegion& region) override;

            virtual std::shared_ptr<IEvent> UploadAsync(IQueue& queue,
                                                        const void* srcData,
                                                        size_t srcBytes,
                                                        const ImageRegion& region,
                                                        const ImageLayout& layout = {}) override;

            virtual std::shared_ptr<IEvent> DownloadAsync(IQueue& queue,
                                                          void* dstData,
                                                          size_t dstBytes,
                                                          const ImageRegion& region,
                                                          const ImageLayout& layout = {}) override;

            virtual std::shared_ptr<IEvent> FillAsync(IQueue& queue,
                                                      const void* value,
                                                      size_t valueSize,
                                                      const ImageRegion& region) override;

            virtual NativeHandle GetNativeHandle() const override
            {
                return { Backend::OpenCL, reinterpret_cast<void*>(ImageObject) };
            }

            cl_mem GetCLImage() const { return ImageObject; }

            static cl_image_format ToCLImageFormat(PixelFormat format);
            static cl_image_desc ToCLImageDesc(const ImageDescription& desc);
        private:
            size_t GetRequiredBytes(const ImageRegion& region,
                                    const ImageLayout& layout) const;
        private:
            std::weak_ptr<IContext> ContextWeak;
            ImageDescription mDescription;
            cl_mem ImageObject = nullptr;
        };
    }
}