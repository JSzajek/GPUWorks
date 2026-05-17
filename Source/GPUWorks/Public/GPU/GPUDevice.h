#pragma once

#include "GpuTypes.h"
#include "GpuCapabilities.h"

namespace Gpu
{
    class IDevice
    {
    public:
        virtual ~IDevice() = default;

        virtual Backend GetBackend() const = 0;
        virtual std::string GetName() const = 0;
        virtual Capabilities GetCapabilities() const = 0;
    };
}