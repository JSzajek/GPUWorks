#pragma once

#include "GPUTypes.h"

#include <memory>
#include <string>

namespace Gpu
{
    class IKernel;

    class IProgram
    {
    public:
        virtual ~IProgram() = default;

        virtual Backend GetBackend() const = 0;
        virtual std::shared_ptr<IKernel> CreateKernel(const std::string& name) = 0;
    };
}