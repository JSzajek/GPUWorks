#pragma once

#include "GpuTypes.h"

namespace Gpu
{
    struct NativeHandle
    {
        Backend Type = Backend::Unknown;
        void* Handle = nullptr;
    };

    class INativeHandleProvider
    {
    public:
        virtual ~INativeHandleProvider() = default;
        virtual NativeHandle GetNativeHandle() const = 0;
    };
}