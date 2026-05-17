// Backends/OpenCL/OpenCLKernel.h
#pragma once

#include "GPU/GPUKernel.h"
#include "Backends/OpenCL/OpenCLCommon.h"

#include <string>

namespace Gpu::OpenCL
{
    class Buffer;
    class Image;

    class Kernel final : public IKernel
    {
        friend Buffer;
    public:
        Kernel(cl_kernel inKernel,
               std::string_view inName);

        virtual ~Kernel() override;

        virtual Backend GetBackend() const override { return Backend::OpenCL; }
        virtual const std::string& GetName() const override { return mName; }

        virtual bool IsValid() const override { return mIsValid.load(); }

        virtual bool SetValueArg(uint32_t index,
                                 const void* data,
                                 size_t size) override;

        virtual bool SetBufferArg(uint32_t index,
                                  IBuffer& buffer) override;

        virtual bool SetImageArg(uint32_t index,
                                 IImage& image) override;

        cl_kernel GetCLKernel() const { return mpKernelHandle; }

    private:
        cl_kernel mpKernelHandle = nullptr;
        std::string mName;
		std::atomic<bool> mIsValid = false;
    };
}