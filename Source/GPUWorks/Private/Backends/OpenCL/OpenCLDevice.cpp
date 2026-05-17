#include "Backends/OpenCL/OpenCLDevice.h"

namespace Gpu::OpenCL
{
    static std::string QueryDeviceString(cl_device_id device, cl_device_info param)
    {
        size_t size = 0;
        clGetDeviceInfo(device, param, 0, nullptr, &size);
        std::string out(size, '\0');
        clGetDeviceInfo(device, param, size, out.data(), nullptr);
        if (!out.empty() && out.back() == '\0')
        {
            out.pop_back();
        }
        return out;
    }

    template<typename T>
    static T QueryDeviceValue(cl_device_id device, cl_device_info param, T fallback = {})
    {
        T value{};
        if (clGetDeviceInfo(device, param, sizeof(T), &value, nullptr) != CL_SUCCESS)
        {
            return fallback;
        }
        return value;
    }

    Device::Device(cl_platform_id inPlatform, cl_device_id inDevice)
        : Platform(inPlatform)
        , DeviceId(inDevice)
    {
        Name = QueryDeviceString(DeviceId, CL_DEVICE_NAME);

        char extensions[1024];
		clGetDeviceInfo(DeviceId, CL_DEVICE_EXTENSIONS, sizeof(extensions), extensions, NULL);
        Caps.bSupportHalfFloat = strstr(extensions, "cl_khr_fp16") != nullptr;

        Caps.bSupportsImages = QueryDeviceValue<cl_bool>(DeviceId, CL_DEVICE_IMAGE_SUPPORT, CL_FALSE) == CL_TRUE;
        Caps.bSupportsRuntimeCompilation = true;
        Caps.ComputeUnitCount = QueryDeviceValue<cl_uint>(DeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, 0);
        Caps.MaxWorkGroupSize = static_cast<uint32_t>(QueryDeviceValue<size_t>(DeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, 0));
        Caps.GlobalMemoryBytes = QueryDeviceValue<cl_ulong>(DeviceId, CL_DEVICE_GLOBAL_MEM_SIZE, 0);

        cl_device_svm_capabilities capabilities = QueryDeviceValue<cl_device_svm_capabilities>(DeviceId, CL_DEVICE_SVM_CAPABILITIES);

        bool bHasCoarseGrainSupport = capabilities & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER;
        bool bHasFineGrainSupport = capabilities & CL_DEVICE_SVM_FINE_GRAIN_BUFFER;
        if (!bHasCoarseGrainSupport && !bHasFineGrainSupport)
        {
            Caps.mSVMSupport = SVMSupport::None;
        }
        else
        {
			Caps.mSVMSupport = bHasFineGrainSupport ? SVMSupport::Fine : SVMSupport::Coarse;
        }
    }
}