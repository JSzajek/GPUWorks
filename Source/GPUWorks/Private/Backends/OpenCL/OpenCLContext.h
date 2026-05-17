#pragma once

#include <memory>
#include <unordered_set>

#include "GPU/GPUContext.h"

namespace Gpu
{
    class IDevice;
    class IQueue;
    class IBuffer;
    class IImage;
    class IProgram;

    namespace OpenCL
    {
        class GPUWORKS_API Context final : public IContext,
                              public std::enable_shared_from_this<Context>
        {
        public:
            Context(cl_context inContext,
                    const std::shared_ptr<IDevice>& inDevice);
            virtual ~Context() override;

            virtual Backend GetBackend() const override { return Backend::OpenCL; }
            virtual std::shared_ptr<IDevice> GetDevice() const override;

            cl_context GetCLContext() const { return mpContextHandle; }

            virtual std::shared_ptr<IQueue> CreateQueue() override;
            virtual std::shared_ptr<IBuffer> CreateBuffer(const BufferDescription& desc) override;
            virtual std::shared_ptr<IImage> CreateImage(const ImageDescription& desc) override;
            virtual std::shared_ptr<IProgram> CreateProgramFromSource(const std::string& source,
                                                                      std::string* outBuildLog = nullptr) override;

            virtual std::shared_ptr<IProgram> CreateProgramFromFile(const std::filesystem::path& filepath,
                                                                    std::string* outBuildLog = nullptr) override;
        private:
            bool ReadProgram(std::string& output,
                             const std::filesystem::path& file,
                             std::unordered_set<std::string>& includedFiles,
							 std::string* outBuildLog = nullptr);
        private:
            cl_context mpContextHandle = nullptr;
            std::shared_ptr<IDevice> mpDevice;
        };
    }
}