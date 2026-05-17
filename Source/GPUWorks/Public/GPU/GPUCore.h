#pragma once

#include "GpuTypes.h"
#include "GpuCapabilities.h"

namespace Gpu
{
    class IDevice;
    class IContext;

    class ICore
    {
    public:
        virtual ~ICore() = default;

        virtual Backend GetBackend() const = 0;
        virtual uint32_t GetDeviceCount() const = 0;
        virtual std::shared_ptr<IDevice> GetDevice(uint32_t index) = 0;
        virtual std::shared_ptr<IContext> CreateContext(std::shared_ptr<IDevice> device) = 0;
    };
}