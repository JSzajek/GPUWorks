#pragma once

#include "GPU/GPUProgram.h"
#include "Backends/OpenCL/OpenCLCommon.h"

namespace Gpu::OpenCL
{
    class Context;

    class Program final : public IProgram
    {
    public:
        Program(std::shared_ptr<Context> inContext, cl_program inProgram);
        virtual ~Program() override;

        virtual Backend GetBackend() const override { return Backend::OpenCL; }
        virtual std::shared_ptr<IKernel> CreateKernel(const std::string& name) override;

        cl_program GetCLProgram() const { return ProgramHandle; }

    private:
        std::weak_ptr<Context> ContextWeak;
        cl_program ProgramHandle = nullptr;
    };
}