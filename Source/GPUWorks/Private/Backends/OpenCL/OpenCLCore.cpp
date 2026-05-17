#include "Backends/OpenCL/OpenCLCore.h"
#include "Backends/OpenCL/OpenCLDevice.h"
#include "Backends/OpenCL/OpenCLContext.h"

namespace Gpu::OpenCL
{
    std::shared_ptr<Core> Core::TryCreate()
    {
        std::shared_ptr<Core> core = std::shared_ptr<Core>(new Core());

        int32_t err = 0;

        // Identify the platforms -----------------------------------------------------------------
        cl_uint numPlatforms = 0;
        clGetPlatformIDs(0, nullptr, &numPlatforms);
        if (numPlatforms == 0)
        {
            return nullptr;
        }

        std::vector<cl_platform_id> platforms(numPlatforms, nullptr);
        err = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);
        if (err < 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Couldn't Identify a Platform!"));
            return nullptr;
        }
        // ----------------------------------------------------------------------------------------

        // Enumerate devices ----------------------------------------------------------------------

        for (cl_platform_id platform : platforms)
        {
            cl_device_id dev = nullptr;

            cl_uint numDevices = 0;
            clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
            if (numDevices == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Couldn't Find a GPU and Falling Back to CPU."));

                err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
                if (err < 0)
                {
                    UE_LOG(LogTemp, Error, TEXT("Couldn't Access Any Devices!"));
                    return nullptr;
                }
            }

            std::vector<cl_device_id> devices(numDevices, nullptr);
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices.data(), NULL);
            if (err < 0)
            {
                UE_LOG(LogTemp, Error, TEXT("Couldn't Retrieve Devices!"));
                return nullptr;
            }

            for (cl_device_id device : devices)
            {
                core->Devices.push_back(std::make_shared<OpenCL::Device>(platform, device));
			}
        }
        // ----------------------------------------------------------------------------------------

        return core;
    }

    uint32_t Core::GetDeviceCount() const
    {
        return static_cast<uint32_t>(Devices.size());
    }

    std::shared_ptr<IDevice> Core::GetDevice(uint32_t index)
    {
        if (index >= Devices.size())
        {
            return nullptr;
        }

        return Devices[index];
    }

    std::shared_ptr<IContext> Core::CreateContext(std::shared_ptr<IDevice> device)
    {
        // Downcast to OpenCL::Device, create cl_context, wrap in OpenCL::Context
        OpenCL::Device* clDevice = reinterpret_cast<OpenCL::Device*>(device.get());
        if (!clDevice)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Device Type!"));
            return nullptr;
        }

        cl_platform_id platformId = clDevice->GetPlatform();
        cl_device_id deviceId = clDevice->GetCLDevice();

        cl_int err = CL_SUCCESS;
        cl_context_properties properties[] =
        {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)platformId,
            0
        };

        cl_context contextHandle = clCreateContext(properties, 1, &deviceId, nullptr, nullptr, &err);
        if (err != CL_SUCCESS || !contextHandle)
        {
	        UE_LOG(LogTemp, Error, TEXT("Failed to Create Context!"));
            return nullptr;
        }
		return std::make_shared<OpenCL::Context>(contextHandle, device);
    }
}