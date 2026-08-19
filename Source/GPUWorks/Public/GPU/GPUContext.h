#pragma once

#include "GPU/GPUTypes.h"

#include <string>
#include <filesystem>

namespace Gpu
{
    class IQueue;
    class IBuffer;
    class IImage;
    class IProgram;
    class IDevice;

    /// <summary>
    /// Buffer description structure used for creating GPU buffers.
    /// </summary>
    struct BufferDescription
    {
        size_t mSizeBytes = 0;

        Access mAccessMode = Access::ReadWrite;

        MemoryUsage mUsage = MemoryUsage::Default;

        BufferSyncMode mSyncMode = BufferSyncMode::Auto;

        const void* mpInitialData = nullptr;
        
        // Optional Behavior Flags ------------------------

        // Allow backend to fallback to a compatible memory type if the requested one isn't supported
        bool mAllowBackendFallback = true;
        // ------------------------------------------------
    };

    /// <summary>
	/// Image description structure used for creating GPU images.
    /// </summary>
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

    /// <summary>
	/// Context interface for managing GPU resources and operations.
    /// </summary>
    class IContext
    {
    public:
        /// <summary>
		/// Default destructor for the IContext interface.
        /// </summary>
        virtual ~IContext() = default;

		/// <summary>
		/// Retrieves the backend type associated with this GPU context.
        /// </summary>
		/// <returns>The backend type of the GPU context</returns>
        virtual Backend GetBackend() const = 0;
        
        /// <summary>
		/// Gets the GPU device associated with this context.
        /// </summary>
		/// <returns>The GPU device</returns>
        virtual std::shared_ptr<IDevice> GetDevice() const = 0;

        /// <summary>
		/// Creates a GPU queue for submitting commands to the GPU.
        /// </summary>
        /// <returns>The GPU queue</returns>
        virtual std::shared_ptr<IQueue> CreateQueue() = 0;
        
        /// <summary>
		/// Creates a GPU buffer with the specified description.
        /// </summary>
        /// <param name="desc">The buffer description</param>
        /// <returns>The GPU buffer</returns>
        virtual std::shared_ptr<IBuffer> CreateBuffer(const BufferDescription& desc) = 0;
        
        /// <summary>
		/// Creates a GPU image with the specified description.
        /// </summary>
        /// <param name="desc">The image description</param>
        /// <returns>The GPU image</returns>
        virtual std::shared_ptr<IImage> CreateImage(const ImageDescription& desc) = 0;
        
        /// <summary>
		/// Creates a GPU program from the provided source code.
        /// </summary>
        /// <param name="desc">The source code string</param>
        /// <param name="outBuildLog">The optional output build log</param>
        /// <returns>The GPU program</returns>
        virtual std::shared_ptr<IProgram> CreateProgramFromSource(const std::string& source,
                                                                  std::string* outBuildLog = nullptr) = 0;

        /// <summary>
        /// Creates a GPU program from the provided source file path.
        /// </summary>
        /// <param name="filepath">The source file path</param>
        /// <param name="outBuildLog">The optional output build log</param>
        /// <returns>The GPU program</returns>
        virtual std::shared_ptr<IProgram> CreateProgramFromFile(const std::filesystem::path& filepath,
                                                                std::string* outBuildLog = nullptr) = 0;
    };
}