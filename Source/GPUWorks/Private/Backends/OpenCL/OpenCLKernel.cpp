#include "Backends/OpenCL/OpenCLKernel.h"
#include "Backends/OpenCL/OpenCLBuffer.h"
#include "Backends/OpenCL/OpenCLImage.h"

namespace Gpu::OpenCL
{
    Kernel::Kernel(cl_kernel kernel,
                   std::string_view name)
        : mpKernelHandle(kernel),
        mName(name),
        mIsValid(true)
    {
    }

    Kernel::~Kernel()
    {
        if (mpKernelHandle)
        {
            clReleaseKernel(mpKernelHandle);
            mpKernelHandle = nullptr;
        }
    }

    bool Kernel::SetValueArg(uint32_t index, const void* data, size_t size)
    {
        bool result = clSetKernelArg(mpKernelHandle, index, size, data) == CL_SUCCESS;
        mIsValid = mIsValid && result;
        return result;
    }

    bool Kernel::SetBufferArg(uint32_t index, IBuffer& bufferBase)
    {
        OpenCL::Buffer* buffer = reinterpret_cast<OpenCL::Buffer*>(&bufferBase);
        if (!buffer)
        {
            return false;
        }

        bool result = buffer->AttachToKernel(*this, index);

        mIsValid = mIsValid && result;
        return result;
    }

    bool Kernel::SetImageArg(uint32_t index, IImage& imageBase)
    {
        OpenCL::Image* image = reinterpret_cast<OpenCL::Image*>(&imageBase);
        if (!image)
        {
            return false;
        }

        cl_mem mem = image->GetCLImage();
        bool result = clSetKernelArg(mpKernelHandle, index, sizeof(cl_mem), &mem) == CL_SUCCESS;

        mIsValid = mIsValid && result;
        return result;
    }
}