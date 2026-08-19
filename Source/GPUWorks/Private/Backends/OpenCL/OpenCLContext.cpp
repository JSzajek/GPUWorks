#include "Backends/OpenCL/OpenCLContext.h"

#include "Backends/OpenCL/OpenCLImage.h"
#include "Backends/OpenCL/OpenCLDevice.h"
#include "Backends/OpenCL/OpenCLProgram.h"
#include "Backends/OpenCL/OpenCLBuffer.h"
#include "Backends/OpenCL/OpenCLQueue.h"

#include <sstream>
#include <fstream>

namespace Gpu::OpenCL
{
    Context::Context(cl_context inContext,
                     const std::shared_ptr<IDevice>& inDevice)
        : mpContextHandle(inContext)
        , mpDevice(inDevice)
    {
    }

    Context::~Context()
    {
        if (mpContextHandle)
        {
            clReleaseContext(mpContextHandle);
            mpContextHandle = nullptr;
        }
    }

    std::shared_ptr<IDevice> Context::GetDevice() const
    {
        return mpDevice;
    }

    std::shared_ptr<IImage> Context::CreateImage(const ImageDescription& desc)
    {
        cl_int err = CL_SUCCESS;
        cl_image_format fmt = Image::ToCLImageFormat(desc.Format);
        cl_image_desc clDesc = Image::ToCLImageDesc(desc);

        cl_mem image = clCreateImage(mpContextHandle,
                                     ToCLMemFlags(desc.AccessMode),
                                     &fmt,
                                     &clDesc,
                                     nullptr,
                                     &err);

        if (err != CL_SUCCESS || !image)
        {
            return nullptr;
        }

        return std::make_shared<Image>(shared_from_this(), desc, image);
    }

	std::shared_ptr<Gpu::IQueue> Context::CreateQueue()
	{
        OpenCL::Device* device = reinterpret_cast<OpenCL::Device*>(mpDevice.get());
        cl_device_id deviceId = device->GetCLDevice();

    #if GPUWORKS_PROFILING
        const cl_command_queue_properties props[] =
        {
            CL_QUEUE_PROPERTIES,
            CL_QUEUE_PROFILING_ENABLE,
            0
        };
    #endif

        cl_int err = CL_SUCCESS;
    #if CL_TARGET_OPENCL_VERSION >= 200
        cl_command_queue queue = clCreateCommandQueueWithProperties(mpContextHandle,
                                                                    deviceId,
                                                                #if GPUWORKS_PROFILING
                                                                    props,
                                                                #else
                                                                    nullptr,
                                                                #endif
                                                                    &err);
    #else
        cl_command_queue queue = clCreateCommandQueue(mpContextHandle,
                                                      deviceId,
                                                      0,
                                                      &err);
    #endif
        if (err != CL_SUCCESS || !queue)
        {
            return nullptr;
        }

        return std::make_shared<Queue>(shared_from_this(), queue);
	}

	std::shared_ptr<Gpu::IBuffer> Context::CreateBuffer(const BufferDescription& desc)
	{
        cl_int err = CL_SUCCESS;
        cl_mem_flags flags = ToCLMemFlags(desc.mAccessMode);

        cl_mem mem = clCreateBuffer(mpContextHandle,
                                    flags,
                                    desc.mSizeBytes,
                                    nullptr,
                                    &err);

        if (err != CL_SUCCESS || !mem)
        {
            return nullptr;
        }
		return std::make_shared<Buffer>(shared_from_this(), desc, mem);;
	}

	std::shared_ptr<Gpu::IProgram> Context::CreateProgramFromSource(const std::string& source,
                                                                    std::string* outBuildLog)
	{
        const char* src = source.c_str();
        const size_t len = source.size();

        cl_int err = CL_SUCCESS;
        cl_program program = clCreateProgramWithSource(mpContextHandle, 1, &src, &len, &err);
        if (err != CL_SUCCESS || !program)
        {
            return nullptr;
        }

		OpenCL::Device* device = reinterpret_cast<OpenCL::Device*>(mpDevice.get());
		cl_device_id deviceId = device->GetCLDevice();
        
        err = clBuildProgram(program, 1, &deviceId, nullptr, nullptr, nullptr);
        if (outBuildLog)
        {
            size_t logSize = 0;
            clGetProgramBuildInfo(program,
                                  deviceId,
                                  CL_PROGRAM_BUILD_LOG,
                                  0,
                                  nullptr,
                                  &logSize);

            outBuildLog->resize(logSize);

            clGetProgramBuildInfo(program,
                                  deviceId,
                                  CL_PROGRAM_BUILD_LOG,
                                  logSize,
                                  outBuildLog->data(),
                                  nullptr);
        }

        if (err != CL_SUCCESS)
        {
            clReleaseProgram(program);
            return nullptr;
        }

        return std::make_shared<OpenCL::Program>(shared_from_this(), program);
	}

	std::shared_ptr<Gpu::IProgram> Context::CreateProgramFromFile(const std::filesystem::path& filepath,
                                                                  std::string* outBuildLog)
	{
        
        std::unordered_set<std::string> includedFiles;
        std::string program_string;
        if (!ReadProgram(program_string,
                         filepath,
                         includedFiles,
                         outBuildLog))
        {
            return nullptr;
        }
        return CreateProgramFromSource(program_string, outBuildLog);
	}

	bool Context::ReadProgram(std::string& output,
                              const std::filesystem::path& file,
                              std::unordered_set<std::string>& includedFiles,
                              std::string* outBuildLog)
	{
        if (!std::filesystem::exists(file))
		{
            if (outBuildLog)
            {
				*outBuildLog = "File Couldn't Be Found At: " + file.string();
            }
            else
            {
				UE_LOG(LogTemp, Error, TEXT("File Couldn't Be Found At: %s"), *FString(file.c_str()));
            }
			return false;
		}

		/* Read program file and place content into buffer */
		std::ifstream program_file;
		program_file.open(file);

		if (!program_file.is_open()) 
		{
            if (outBuildLog)
            {
				*outBuildLog = "Could Not Open File: " + file.string();
            }
            else
            {
				UE_LOG(LogTemp, Error, TEXT("Could Not Open File: %s"), *FString(file.c_str()));
            }
			return false;
		}

		std::stringstream buffer;
		buffer << program_file.rdbuf();
        output = buffer.str();
		if (output.find("#include") != std::string::npos)
		{
			std::istringstream iss(output);
			std::string line;
			while (std::getline(iss, line))
			{
				if (line.find("#include") != std::string::npos)
				{
					std::string includeFile = line.substr(line.find_first_of("\"") + 1, line.find_last_of("\"") - line.find_first_of("\"") - 1);
					if (includedFiles.find(includeFile) == includedFiles.end())
					{
						includedFiles.insert(includeFile);
						std::filesystem::path includeFilePath = file.parent_path() / includeFile;

						std::string includeFileContent;
                        ReadProgram(includeFileContent, 
                                    includeFilePath,
                                    includedFiles,
                                    outBuildLog);

                        output.replace(output.find(line), line.size(), includeFileContent);
					}
				}
			}
		}
		return true;
	}
}