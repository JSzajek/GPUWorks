#pragma once

#include "GpuCore.h"
#include "GpuTypes.h"

#include <memory>

namespace Gpu
{
    /// <summary>
	/// Describes the preferences for creating a GPU core instance.
    /// </summary>
    struct FactoryDesc
    {
        Backend mPreferredBackend = Backend::OpenCL;
        bool mAllowFallback = true;
        int32_t mPreferredDeviceIndex = 0;
    };

    /// <summary>
	/// Factory class for creating GPU core instances based on the specified backend or preferences.
    /// </summary>
    class GPUWORKS_API Factory
    {
    public:
        /// <summary>
		/// Creates a GPU core instance for the specified backend.
        /// </summary>
		/// <param name="backend">The backend to use for creating the GPU core instance</param>
        /// <returns>A shared pointer to the created GPU core instance</returns>
        static std::shared_ptr<ICore> Create(Backend backend);

        /// <summary>
		/// Creates a GPU core instance based on the specified factory description.
        /// </summary>
        /// <param name="desc">The factory description containing preferences for creating the GPU core instance</param>
        /// <returns>A shared pointer to the created GPU core instance</returns>
        static std::shared_ptr<ICore> Create(const FactoryDesc& desc);

        /// <summary>
		/// Checks whether the specified backend is available on the system.
        /// </summary>
        /// <param name="backend">The backend to check for availability</param>
        /// <returns>True if the backend is available, false otherwise</returns>
        static bool IsBackendAvailable(Backend backend);

        /// <summary>
		/// Retrieves the available backends on the system.
        /// </summary>
        /// <returns>The available backends</returns>
        static std::vector<Backend> GetAvailableBackends();
    private:
        /// <summary>
		/// Creates a GPU core instance for the OpenCL backend.
        /// </summary>
        /// <returns>A shared pointer to the created GPU core instance</returns>
        static std::shared_ptr<ICore> CreateOpenCL();

        /// <summary>
		/// Creates a GPU core instance for the CUDA backend.
        /// </summary>
        /// <returns>A shared pointer to the created GPU core instance</returns>
        static std::shared_ptr<ICore> CreateCUDA();
    };
}