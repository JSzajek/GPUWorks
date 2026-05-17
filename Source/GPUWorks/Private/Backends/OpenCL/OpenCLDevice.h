#pragma once

#include "GPU/GPUDevice.h"

#include "Backends/OpenCL/OpenCLCommon.h"

namespace Gpu::OpenCL
{
    class GPUWORKS_API Device final : public IDevice
    {
    public:
        Device(cl_platform_id inPlatform, cl_device_id inDevice);

        virtual Backend GetBackend() const override { return Backend::OpenCL; }
        virtual std::string GetName() const override { return Name; }
        virtual Capabilities GetCapabilities() const override { return Caps; }

        cl_platform_id GetPlatform() const { return Platform; }
        cl_device_id GetCLDevice() const { return DeviceId; }
    private:
        cl_platform_id Platform = nullptr;
        cl_device_id DeviceId = nullptr;
        std::string Name;
        Capabilities Caps;
    };
}