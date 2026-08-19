#pragma once

#include "GpuTypes.h"
#include "GpuCapabilities.h"

namespace Gpu
{
    /// <summary>
	/// Interface for a GPU device.
    /// </summary>
    class IDevice
    {
    public:
        /// <summary>
		/// Default destructor.
        /// </summary>
        virtual ~IDevice() = default;
        
        /// <summary>
		/// Retrieves the backend of the GPU device.
        /// </summary>
        /// <returns>The backend of the GPU device</returns>
        virtual Backend GetBackend() const = 0;

		/// <summary>
		/// Retrieves the name of the GPU device.
        /// </summary>
        /// <returns>The name of the GPU device</returns>
        virtual std::string GetName() const = 0;

        /// <summary>
		/// Retrieves the capabilities of the GPU device.
        /// </summary>
        /// <returns>The capabilities of the GPU device</returns>
        virtual Capabilities GetCapabilities() const = 0;
    };
}