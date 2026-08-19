#pragma once

#include "GPUTypes.h"

#include <memory>
#include <string>

namespace Gpu
{
    class IKernel;

    /// <summary>
	/// Interface for a GPU program, which represents a compiled GPU program that can create kernels.
    /// </summary>
    class IProgram
    {
    public:
        /// <summary>
        /// Default destructor.
        /// </summary>
        virtual ~IProgram() = default;

        /// <summary>
		/// Retrieves the backend type associated with this GPU program.
        /// </summary>
        /// <returns></returns>
        virtual Backend GetBackend() const = 0;

        /// <summary>
		/// Creates a kernel from the GPU program with the specified name.
        /// </summary>
        /// <param name="name">The name of the kernel</param>
        /// <returns>The created kernel</returns>
        virtual std::shared_ptr<IKernel> CreateKernel(const std::string& name) = 0;
    };
}