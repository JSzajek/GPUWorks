#pragma once

#include "GpuTypes.h"
#include "GpuCapabilities.h"

#include <memory>

namespace Gpu
{
    class IDevice;
    class IContext;

    /// <summary>
	/// Interface for the GPU core, which provides access to GPU devices and contexts.
    /// </summary>
    class ICore
    {
    public:
        /// <summary>
        /// Default destructor.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        virtual ~ICore() = default;

        /// <summary>
        /// Retrieves the backend of the GPU device.
        /// </summary>
        /// <returns>The backend of the GPU device</returns>
        virtual Backend GetBackend() const = 0;

        /// <summary>
		/// Retrieves the number of available GPU devices.
        /// </summary>
        /// <returns>The number of available GPU devices</returns>
        virtual uint32_t GetDeviceCount() const = 0;

        /// <summary>
        /// Retrieves a GPU device by its index.
        /// </summary>
        /// <param name="index">The index of the GPU device</param>
        /// <returns>The GPU device at the specified index</returns>
        virtual std::shared_ptr<IDevice> GetDevice(uint32_t index) = 0;

        /// <summary>
        /// Creates a GPU context for the specified device.
        /// </summary>
        /// <param name="device">The GPU device for which to create the context</param>
        /// <returns>The created GPU context</returns>
        virtual std::shared_ptr<IContext> CreateContext(std::shared_ptr<IDevice> device) = 0;
    };
}