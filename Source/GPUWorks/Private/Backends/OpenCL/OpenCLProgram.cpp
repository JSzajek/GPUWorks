#include "Backends/OpenCL/OpenCLProgram.h"
#include "Backends/OpenCL/OpenCLKernel.h"

namespace Gpu::OpenCL
{
    Program::Program(std::shared_ptr<Context> inContext, cl_program inProgram)
        : ContextWeak(inContext)
        , ProgramHandle(inProgram)
    {
    }

    Program::~Program()
    {
        if (ProgramHandle)
        {
            clReleaseProgram(ProgramHandle);
            ProgramHandle = nullptr;
        }
    }

    std::shared_ptr<IKernel> Program::CreateKernel(const std::string& name)
    {
        cl_int err = CL_SUCCESS;
        cl_kernel kernel = clCreateKernel(ProgramHandle, name.c_str(), &err);
        if (err != CL_SUCCESS || !kernel)
        {
            return nullptr;
        }

        return std::make_shared<Kernel>(kernel, name);
    }
}