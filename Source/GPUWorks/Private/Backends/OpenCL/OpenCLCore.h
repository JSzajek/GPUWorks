#pragma once

#include "GPU/GPUCore.h"

#include <OpenCLLib.h>

namespace Gpu::OpenCL
{
    class Core final : public ICore
    {
    public:
        static std::shared_ptr<Core> TryCreate();

        virtual Backend GetBackend() const override { return Backend::OpenCL; }
        virtual uint32_t GetDeviceCount() const override;
        virtual std::shared_ptr<IDevice> GetDevice(uint32_t index) override;
        virtual std::shared_ptr<IContext> CreateContext(std::shared_ptr<IDevice> device) override;
    private:
        Core() = default;
    private:
        std::vector<std::shared_ptr<IDevice>> Devices;
    };
}