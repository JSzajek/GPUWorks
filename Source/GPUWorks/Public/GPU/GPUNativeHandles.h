#pragma once

#include "GpuTypes.h"

namespace Gpu
{
    /// <summary>
	/// Native handle structure that contains the backend type and the native handle pointer.
    /// </summary>
    struct NativeHandle
    {
        Backend mType = Backend::Unknown;
        void* mpHandle = nullptr;
    };

    /// <summary>
	/// Interface for classes that provide a native handle.
    /// </summary>
    class INativeHandleProvider
    {
    public:
        /// <summary>
        /// Default destructor.
        /// </summary>
        virtual ~INativeHandleProvider() = default;

        /// <summary>
		/// Retrieves the native handle associated with the object.
        /// </summary>
        /// <returns>The native handle</returns>
        virtual NativeHandle GetNativeHandle() const = 0;
    };
}