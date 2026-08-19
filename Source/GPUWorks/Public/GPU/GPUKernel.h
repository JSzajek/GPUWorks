#pragma once

#include "GPUTypes.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace Gpu
{
    class IBuffer;
    class IImage;

    /// <summary>
	/// Interface for a GPU kernel, representing a compute
    /// shader or function that can be executed on the GPU.
    /// </summary>
    class IKernel
    {
    public:
        /// <summary>
		/// Default destructor.
        /// </summary>
        virtual ~IKernel() = default;

        /// <summary>
		/// Get the backend type of the kernel.
        /// </summary>
        virtual Backend GetBackend() const = 0;

        /// <summary>
		/// Get the name of the kernel.
        /// </summary>
        virtual const std::string& GetName() const = 0;

		/// <summary>
		/// Check if the kernel is valid and ready for execution.
		/// </summary>
		virtual bool IsValid() const = 0;

        /// <summary>
		/// Set the value of a kernel argument at the specified index.
        /// </summary>
        /// <param name="index">The index of the kernel argument to set.</param>
        /// <param name="data">A pointer to the data to set for the argument.</param>
        /// <param name="size">The size of the data in bytes.</param>
        /// <returns>True if the argument was successfully set, false otherwise.</returns>
        virtual bool SetValueArg(uint32_t index,
                                 const void* data,
                                 size_t size) = 0;
        
        /// <summary>
		/// Set the buffer argument for the kernel at the specified index.
        /// </summary>
        /// <param name="index">The index of the kernel argument to set.</param>
        /// <param name="buffer">The buffer to set as the argument.</param>
        /// <returns>True if the argument was successfully set, false otherwise.</returns>
        virtual bool SetBufferArg(uint32_t index,
                                  IBuffer& buffer) = 0;

        /// <summary>
		/// Set the image argument for the kernel at the specified index.
        /// </summary>
        /// <param name="index">The index of the kernel argument to set.</param>
        /// <param name="image">The image to set as the argument.</param>
        /// <returns>True if the argument was successfully set, false otherwise.</returns>
        virtual bool SetImageArg(uint32_t index,
                                 IImage& image) = 0;

        /// <summary>
		/// Set the value of a kernel argument at the specified index.
        /// </summary>
        /// <typeparam name="T">The type of value argument</typeparam>
        /// <param name="index">The index of the kernel argument to set.</param>
        /// <param name="value">The value to set as the argument.</param>
        /// <returns>True if the argument was successfully set, false otherwise.</returns>
        template<typename T>
        bool SetValueArg(uint32_t index,
                         const T& value)
        {
            return SetValueArg(index, &value, sizeof(T));
        }
    };
}