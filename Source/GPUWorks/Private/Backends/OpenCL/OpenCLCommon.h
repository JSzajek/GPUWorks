#pragma once

#include "OpenCLLib.h"

#include <memory>
#include <string>
#include <vector>

#include "Private/GPUWorksLog.h"

#ifndef GPU_OPENCL_CHECK
#define GPU_OPENCL_CHECK(expr) \
    do { \
        const cl_int _err = (expr); \
        if (_err != CL_SUCCESS) { return false; } \
    } while (0)
#endif

namespace Gpu::OpenCL
{
    inline std::string GetErrorString(cl_int err)
    {
        switch (err)
        {
            case CL_SUCCESS:
                return "CL_SUCCESS";
            case CL_DEVICE_NOT_FOUND:
                return "CL_DEVICE_NOT_FOUND";
            case CL_DEVICE_NOT_AVAILABLE:
                return "CL_DEVICE_NOT_AVAILABLE";
            case CL_COMPILER_NOT_AVAILABLE:
                return "CL_COMPILER_NOT_AVAILABLE";
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
            case CL_OUT_OF_RESOURCES:
                return "CL_OUT_OF_RESOURCES";
            case CL_OUT_OF_HOST_MEMORY:
                return "CL_OUT_OF_HOST_MEMORY";
            case CL_BUILD_PROGRAM_FAILURE:
                return "CL_BUILD_PROGRAM_FAILURE";
            case CL_INVALID_VALUE:
                return "CL_INVALID_VALUE";
            case CL_INVALID_DEVICE:
                return "CL_INVALID_DEVICE";
            case CL_INVALID_CONTEXT:
                return "CL_INVALID_CONTEXT";
            case CL_INVALID_MEM_OBJECT:
                return "CL_INVALID_MEM_OBJECT";
            case CL_INVALID_COMMAND_QUEUE:
                return "CL_INVALID_COMMAND_QUEUE";
            case CL_INVALID_PROGRAM:
                return "CL_INVALID_PROGRAM";
            case CL_INVALID_KERNEL:
                return "CL_INVALID_KERNEL";
            case CL_INVALID_ARG_INDEX:
                return "CL_INVALID_ARG_INDEX";
            case CL_INVALID_ARG_VALUE:
                return "CL_INVALID_ARG_VALUE";
            case CL_INVALID_ARG_SIZE:
                return "CL_INVALID_ARG_SIZE";
            case CL_INVALID_KERNEL_ARGS:
                return "CL_INVALID_KERNEL_ARGS";
            case CL_INVALID_WORK_GROUP_SIZE:
                return "CL_INVALID_WORK_GROUP_SIZE";
            default: return "Unknown OpenCL error";
        }
    }

    inline cl_mem_flags ToCLMemFlags(Gpu::Access access)
    {
        switch (access)
        {
            case Gpu::Access::ReadOnly:
                return CL_MEM_READ_ONLY;
            case Gpu::Access::WriteOnly:
                return CL_MEM_WRITE_ONLY;
            case Gpu::Access::ReadWrite:
                return CL_MEM_READ_WRITE;
            default:
                return CL_MEM_READ_WRITE;
        }
    }

    inline void LogCLError(const std::string& message,
                           cl_int err)
    {
        UE_LOG(LogGPUWorks, Warning, TEXT("%s: %s"), *FString(message.c_str()), *FString(GetErrorString(err).c_str()));
	}
}