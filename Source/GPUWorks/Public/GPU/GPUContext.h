#pragma once

#include "GPU/GPUTypes.h"

#include <filesystem>

namespace Gpu
{
    class IQueue;
    class IBuffer;
    class IImage;
    class IProgram;
    class IDevice;

    struct BufferDescription
    {
        size_t SizeBytes = 0;

        Access AccessMode = Access::ReadWrite;

        MemoryUsage Usage = MemoryUsage::Default;

		BufferSyncMode SyncMode = BufferSyncMode::Auto;

        const void* InitialData = nullptr;
        
        // Optional Behavior Flags ------------------------

        // Allow backend to fallback to a compatible memory type if the requested one isn't supported
		bool AllowBackendFallback = true;
        // ------------------------------------------------
    };

    struct ImageDescription
    {
        ImageType Type = ImageType::Tex2D;
        PixelFormat Format = PixelFormat::RGBA8;

        uint32_t Width = 1;
        uint32_t Height = 1;
        uint32_t DepthOrLayers = 1;

        Access AccessMode = Access::ReadWrite;
        MemoryUsage Usage = MemoryUsage::Default;

        bool bEnableHostReadback = false;
        bool bEnableHostUpload = false;
    };

    class IContext
    {
    public:
        virtual ~IContext() = default;

        virtual Backend GetBackend() const = 0;
        virtual std::shared_ptr<IDevice> GetDevice() const = 0;

        virtual std::shared_ptr<IQueue> CreateQueue() = 0;
        virtual std::shared_ptr<IBuffer> CreateBuffer(const BufferDescription& desc) = 0;
        virtual std::shared_ptr<IImage> CreateImage(const ImageDescription& desc) = 0;
        virtual std::shared_ptr<IProgram> CreateProgramFromSource(const std::string& source,
                                                                  std::string* outBuildLog = nullptr) = 0;

        virtual std::shared_ptr<IProgram> CreateProgramFromFile(const std::filesystem::path& filepath,
                                                                std::string* outBuildLog = nullptr) = 0;
    };
}