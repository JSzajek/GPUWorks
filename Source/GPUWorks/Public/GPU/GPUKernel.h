#pragma once

#include "GPUTypes.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace Gpu
{
    class IBuffer;
    class IImage;

    class IKernel
    {
    public:
        virtual ~IKernel() = default;

        virtual Backend GetBackend() const = 0;
        virtual const std::string& GetName() const = 0;

		virtual bool IsValid() const = 0;

        virtual bool SetValueArg(uint32_t index, const void* data, size_t size) = 0;
        virtual bool SetBufferArg(uint32_t index, IBuffer& buffer) = 0;
        virtual bool SetImageArg(uint32_t index, IImage& image) = 0;

        template<typename T>
        bool SetValueArg(uint32_t index, const T& value)
        {
            return SetValueArg(index, &value, sizeof(T));
        }
    };
}