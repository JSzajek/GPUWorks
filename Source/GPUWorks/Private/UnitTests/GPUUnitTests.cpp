#include "Misc/AutomationTest.h"

#include "Interfaces/IPluginManager.h"

#include "GPUWorksLib.h"

#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DArray.h"

#include "Private/UnitTests/TestUWorld.h"
#include "Kismet/KismetRenderingLibrary.h"

// Reference: https://minifloppy.it/posts/2024/automated-testing-specs-ue5/#writing-tests

BEGIN_DEFINE_SPEC(FGPUUnitTestsSpecs, "CLWorks Unit Test",
				  EAutomationTestFlags::EditorContext | 
				  EAutomationTestFlags::CommandletContext |
				  EAutomationTestFlags::ProductFilter);

// Variables and functions defined here will end up being member of
// the FGPUUnitTestsSpecs class and will be accessible in the tests

FString ModuleDirectory = IPluginManager::Get().FindPlugin("GPUWorks")->GetBaseDir();

TUniquePtr<FTestUWorld> TestWorld = nullptr;

int32 mDefaultUTextureWidth = 256;
int32 mDefaultUTextureHeight = 256;
int32 mDefaultUTextureArraySlices = 4;
int32 mDefaultUTextureDepth = 256;

std::shared_ptr<Gpu::ICore> mpCore = nullptr;
std::shared_ptr<Gpu::IDevice> mpDevice = nullptr;
std::shared_ptr<Gpu::IContext> mpContext = nullptr;

TObjectPtr<UGPUContextObject> mpGPUContextObj = nullptr;

END_DEFINE_SPEC(FGPUUnitTestsSpecs);

void FGPUUnitTestsSpecs::Define()
{
	Describe("Essentials", [this]()
	{
		It("(1) Device Enumeration", [this]()
		{
			Gpu::FactoryDesc desc;
			desc.PreferredBackend = Gpu::Backend::OpenCL;
			desc.bAllowFallback = false;

			std::shared_ptr<Gpu::ICore> core = Gpu::Factory::Create(desc);
			TestTrue(TEXT("Invalid Device!"), core->GetDevice(0) != nullptr);
		});

		It("(2) Context Creation", [this]()
		{
			Gpu::FactoryDesc desc;
			desc.PreferredBackend = Gpu::Backend::OpenCL;
			desc.bAllowFallback = false;

			std::shared_ptr<Gpu::ICore> core = Gpu::Factory::Create(desc);
			std::shared_ptr<Gpu::IDevice> device = core->GetDevice(0);
			std::shared_ptr<Gpu::IContext> context = core->CreateContext(device);

			TestTrue(TEXT("Failed Context Creation!"), context != nullptr);
		});

		It("(3) Command Queue Creation", [this]()
		{
			Gpu::FactoryDesc desc;
			desc.PreferredBackend = Gpu::Backend::OpenCL;
			desc.bAllowFallback = false;

			std::shared_ptr<Gpu::ICore> core = Gpu::Factory::Create(desc);
			std::shared_ptr<Gpu::IDevice> device = core->GetDevice(0);
			std::shared_ptr<Gpu::IContext> context = core->CreateContext(device);

			if (!TestTrue(TEXT("Failed Context Creation!"), context != nullptr))
				return;

			std::shared_ptr<Gpu::IQueue> queue = context->CreateQueue();

			TestTrue(TEXT("Failed Command Queue Creation!"), queue != nullptr);
		});
	});

	Describe("Kernel Setup", [this]()
	{
		BeforeEach([this]()
		{
			Gpu::FactoryDesc desc;
			desc.PreferredBackend = Gpu::Backend::OpenCL;
			desc.bAllowFallback = false;

			mpCore = Gpu::Factory::Create(desc);
			mpDevice = mpCore->GetDevice(0);
			mpContext = mpCore->CreateContext(mpDevice);
		});

		AfterEach([this]()
		{
			mpCore.reset();
			mpDevice.reset();
			mpContext.reset();
		});

		It("(1) Kernel Compilation - String", [this]()
		{
			std::string buildLog;
			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void test() { }", 
																						&buildLog);

			AddInfo(FString::Printf(TEXT("Program BuildLog:\n\t%s"), *FString(buildLog.c_str())));
			TestTrue(TEXT("Invalid Program"), program != nullptr);
		});

		It("(2) Kernel Compilation - File", [this]()
		{
			const std::string moduleDirStr = std::string(TCHAR_TO_UTF8(*ModuleDirectory));

			std::string buildLog;
			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromFile(moduleDirStr + "/UnitTest/Shaders/add_vectors.cl",
																					  &buildLog);

			AddInfo(FString::Printf(TEXT("Program BuildLog:\n\t%s"), *FString(buildLog.c_str())));
			TestTrue(TEXT("Invalid Program"), program != nullptr);
		});
		
		It("(3) Kernel Failure", [this]()
		{
			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void test() { }");
			TestTrue(TEXT("Invalid Program"), program != nullptr);

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("foo");
			TestTrue(TEXT("Invalid Kernel Should Return nullptr"), kernel == nullptr);
		});

		It("(4) Argument Setting", [this]()
		{
			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void test(float a, float b) { }");
			TestTrue(TEXT("Invalid Program"), program != nullptr);

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("test");
			TestTrue(TEXT("Kernel Invalid"), kernel != nullptr);

			kernel->SetValueArg(0, 2.0f);
			TestTrue(TEXT("Failed to Set First Kernel Argument Program"), kernel->IsValid());

			kernel->SetValueArg(1, 11.0f);
			TestTrue(TEXT("Failed to Set Second Kernel Argument Program"), kernel->IsValid());
		});

		It("(5) Invalid Argument", [this]()
		{
			const std::string moduleDirStr = std::string(TCHAR_TO_UTF8(*ModuleDirectory));

			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void test(float a) { }");
			TestTrue(TEXT("Invalid Program"), program != nullptr);

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("test");
			TestTrue(TEXT("Kernel Invalid"), kernel != nullptr);

			kernel->SetValueArg(3, 9.0f);
			TestFalse(TEXT("Set Invalid Kernel Argument"), kernel->IsValid());
		});
	});

	Describe("Buffer Handling", [this]()
	{
		BeforeEach([this]()
		{
			Gpu::FactoryDesc desc;
			desc.PreferredBackend = Gpu::Backend::OpenCL;
			desc.bAllowFallback = false;

			mpCore = Gpu::Factory::Create(desc);
			mpDevice = mpCore->GetDevice(0);
			mpContext = mpCore->CreateContext(mpDevice);
		});
		
		AfterEach([this]()
		{
			mpCore.reset();
			mpDevice.reset();
			mpContext.reset();
		});

		It("(1) Buffer Creation", [this]()
		{
			size_t count = 10;
			std::vector<float> test_input(count, 0.0f);
			std::vector<float> test_inout(count, 0.0f);

			Gpu::BufferDescription bufferDescOut;
			bufferDescOut.SizeBytes = count * sizeof(float);
			bufferDescOut.AccessMode = Gpu::Access::WriteOnly;
			bufferDescOut.SyncMode = Gpu::BufferSyncMode::CopyOnce;
			bufferDescOut.InitialData = test_input.data();
			std::shared_ptr<Gpu::IBuffer> bufferOutput = mpContext->CreateBuffer(bufferDescOut);
			TestTrue(TEXT("Failed Write-Only Buffer Creation!"), bufferOutput != nullptr);

			Gpu::BufferDescription bufferDescIn;
			bufferDescIn.SizeBytes = count * sizeof(float);
			bufferDescIn.AccessMode = Gpu::Access::ReadOnly;
			bufferDescIn.SyncMode = Gpu::BufferSyncMode::Stream;
			std::shared_ptr<Gpu::IBuffer> bufferInput = mpContext->CreateBuffer(bufferDescIn);
			TestTrue(TEXT("Failed Read-Only Buffer Creation!"), bufferInput != nullptr);

			Gpu::BufferDescription bufferDescInOut;
			bufferDescInOut.SizeBytes = count * sizeof(float);
			bufferDescInOut.AccessMode = Gpu::Access::ReadWrite;
			bufferDescInOut.SyncMode = Gpu::BufferSyncMode::Stream;
			bufferDescInOut.InitialData = test_input.data();
			std::shared_ptr<Gpu::IBuffer> bufferInOut = mpContext->CreateBuffer(bufferDescInOut);
			TestTrue(TEXT("Failed Read-Write Buffer Creation!"), bufferInOut != nullptr);
		});

		It("(2) Buffer Read", [this]()
		{
			size_t count = 5;
			std::vector<float> input_data = { 30, 2, 45, 19, 54 };

			Gpu::BufferDescription bufferDescInOut;
			bufferDescInOut.SizeBytes = count * sizeof(float);
			bufferDescInOut.AccessMode = Gpu::Access::ReadWrite;
			bufferDescInOut.SyncMode = Gpu::BufferSyncMode::Stream;
			bufferDescInOut.InitialData = input_data.data();
			std::shared_ptr<Gpu::IBuffer> bufferInOut = mpContext->CreateBuffer(bufferDescInOut);
			TestTrue(TEXT("Failed Read-Write Buffer Creation!"), bufferInOut != nullptr);

			std::vector<float> target_output(count, 0.0f);

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			bufferInOut->Download(*queue, target_output.data(), count * sizeof(float));

			for (size_t i = 0; i < count; ++i)
			{
				std::string msg = (std::to_string(target_output[i]) + " != " + std::to_string(input_data[i]));
				if (!TestTrue(FString(msg.c_str()), target_output[i] == input_data[i]))
				{
					return;
				}
			}
		});

		It("(3) Buffer Read & Write", [this]()
		{
			size_t count = 5;
			std::vector<float> input_data = { 30, 2, 45, 19, 54 };
			std::vector<float> target_output = { 60, 4, 90, 38, 108 };

			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void double_data(__global float* data)\n" 
																						"{ int i = get_global_id(0); \n"
																						"data[i] = data[i] * 2; }");
			TestTrue(TEXT("Invalid Program"), program != nullptr);

			Gpu::BufferDescription bufferDescInOut;
			bufferDescInOut.SizeBytes = count * sizeof(float);
			bufferDescInOut.AccessMode = Gpu::Access::ReadWrite;
			bufferDescInOut.SyncMode = Gpu::BufferSyncMode::Stream;
			bufferDescInOut.InitialData = input_data.data();
			std::shared_ptr<Gpu::IBuffer> bufferInOut = mpContext->CreateBuffer(bufferDescInOut);
			TestTrue(TEXT("Failed Read-Write Buffer Creation!"), bufferInOut != nullptr);

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("double_data");
			if (!TestTrue(TEXT("Kernel Invalid"), kernel != nullptr))
				return;

			kernel->SetBufferArg(0, *bufferInOut);
			if (!TestTrue(TEXT("Failed to Set Buffer Argument Program"), kernel->IsValid()))
				return;

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			TestTrue(TEXT("Failed To Create Queue"), queue != nullptr);

			Gpu::DispatchDescription dispatchDesc;
			dispatchDesc.Dim = 1;
			dispatchDesc.Global[0] = count;
			queue->Dispatch(*kernel, dispatchDesc);
			queue->Finish();

			std::vector<float> output_data(count, 0.0f);
			bufferInOut->Download(*queue, output_data.data(), count * sizeof(float));
			for (size_t i = 0; i < count; ++i)
			{
				std::string msg = (std::to_string(target_output[i]) + " != " + std::to_string(output_data[i]));
				if (!TestTrue(FString(msg.c_str()), target_output[i] == output_data[i]))
					return;
			}
		});

		It("(4) Zero Copy Buffers", [this]()
		{
			size_t count = 5;
			std::vector<float> input_data = { 30, 2, 45, 19, 54 };
			std::vector<float> target_output = { 15, 1, 22.5, 9.5, 27 };

			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void half_data(__global float* data)\n" 
																						"{ int i = get_global_id(0); \n"
																						"data[i] = data[i] * 0.5f; }");
			TestTrue(TEXT("Invalid Program"), program != nullptr);

			Gpu::BufferDescription bufferDescInOut;
			bufferDescInOut.SizeBytes = count * sizeof(float);
			bufferDescInOut.AccessMode = Gpu::Access::ReadWrite;
			bufferDescInOut.SyncMode = Gpu::BufferSyncMode::ZeroCopy;
			bufferDescInOut.InitialData = input_data.data();
			std::shared_ptr<Gpu::IBuffer> bufferInOut = mpContext->CreateBuffer(bufferDescInOut);
			TestTrue(TEXT("Failed Read-Write Buffer Creation!"), bufferInOut != nullptr);

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("half_data");
			TestTrue(TEXT("Kernel Invalid"), kernel != nullptr);

			kernel->SetBufferArg(0, *bufferInOut);
			TestTrue(TEXT("Failed to Set Buffer Argument Program"), kernel->IsValid());

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			TestTrue(TEXT("Failed To Create Queue"), queue != nullptr);

			Gpu::DispatchDescription dispatchDesc;
			dispatchDesc.Dim = 1;
			dispatchDesc.Global[0] = count;
			queue->Dispatch(*kernel, dispatchDesc);
			queue->Finish();

			std::vector<float> output_data(count, 0.0f);
			bufferInOut->Download(*queue, output_data.data(), count * sizeof(float));
			for (size_t i = 0; i < count; ++i)
			{
				std::string msg = (std::to_string(target_output[i]) + " != " + std::to_string(output_data[i]));
				if (!TestTrue(FString(msg.c_str()), target_output[i] == output_data[i]))
					return;
			}
		});

		It("(5) Multi-Buffer Access", [this]()
		{
			//TODO:: Implement
		});
	});

	Describe("Kernel Execution", [this]()
	{
		BeforeEach([this]()
		{
			Gpu::FactoryDesc desc;
			desc.PreferredBackend = Gpu::Backend::OpenCL;
			desc.bAllowFallback = false;

			mpCore = Gpu::Factory::Create(desc);
			mpDevice = mpCore->GetDevice(0);
			mpContext = mpCore->CreateContext(mpDevice);
		});

		
		AfterEach([this]()
		{
			mpCore.reset();
			mpDevice.reset();
			mpContext.reset();
		});

		It("(1) Enqueue", [this]()
		{
			size_t count = 5;
			std::vector<float> input_data(count, 3.0f);

			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void test(__global float* data) { }");
			TestTrue(TEXT("Invalid Program"), program != nullptr);

			Gpu::BufferDescription bufferDescInOut;
			bufferDescInOut.SizeBytes = count * sizeof(float);
			bufferDescInOut.AccessMode = Gpu::Access::ReadWrite;
			bufferDescInOut.SyncMode = Gpu::BufferSyncMode::Stream;
			bufferDescInOut.InitialData = input_data.data();
			std::shared_ptr<Gpu::IBuffer> bufferInOut = mpContext->CreateBuffer(bufferDescInOut);
			TestTrue(TEXT("Failed Read-Write Buffer Creation!"), bufferInOut != nullptr);

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("test");
			if (!TestTrue(TEXT("Kernel Invalid"), kernel != nullptr))
				return;

			kernel->SetBufferArg(0, *bufferInOut);
			if (!TestTrue(TEXT("Failed to Set Buffer Argument Program"), kernel->IsValid()))
				return;

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			if (!TestTrue(TEXT("Failed To Create Queue"), queue != nullptr))
				return;

			Gpu::DispatchDescription dispatchDesc;
			dispatchDesc.Dim = 1;
			dispatchDesc.Global[0] = count;
			std::shared_ptr<Gpu::IEvent> event = queue->Dispatch(*kernel, dispatchDesc);
			if (!TestTrue(TEXT("Failed To Dispatch"), event != nullptr))
				return;

			event->Wait();
		});

		It("(2) Work Sizes", [this]()
		{
			size_t count = 5;
			std::vector<float> input_data = { 30, 2, 45, 19, 54 };

			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void triple_data(__global const float* data, __global float* result)\n" 
																						"{ int i = get_global_id(0); \n"
																						"result[i] = data[i] * 3; }");
			TestTrue(TEXT("Invalid Program"), program != nullptr);

			Gpu::BufferDescription bufferDescIn;
			bufferDescIn.SizeBytes = count * sizeof(float);
			bufferDescIn.AccessMode = Gpu::Access::ReadOnly;
			bufferDescIn.SyncMode = Gpu::BufferSyncMode::CopyOnce;
			bufferDescIn.InitialData = input_data.data();
			std::shared_ptr<Gpu::IBuffer> bufferIn = mpContext->CreateBuffer(bufferDescIn);
			TestTrue(TEXT("Failed Read-Only Buffer Creation!"), bufferIn != nullptr);

			Gpu::BufferDescription bufferDescInOut;
			bufferDescInOut.SizeBytes = count * sizeof(float);
			bufferDescInOut.AccessMode = Gpu::Access::ReadWrite;
			bufferDescInOut.SyncMode = Gpu::BufferSyncMode::Stream;
			bufferDescInOut.InitialData = input_data.data();
			std::shared_ptr<Gpu::IBuffer> bufferInOut = mpContext->CreateBuffer(bufferDescInOut);
			TestTrue(TEXT("Failed Read-Write Buffer Creation!"), bufferInOut != nullptr);

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("triple_data");
			if (!TestTrue(TEXT("Kernel Invalid"), kernel != nullptr))
				return;

			kernel->SetBufferArg(0, *bufferIn);
			if (!TestTrue(TEXT("Failed to Set Input Buffer Argument Program"), kernel->IsValid()))
				return;

			kernel->SetBufferArg(1, *bufferInOut);
			if (!TestTrue(TEXT("Failed to Set Output Buffer Argument Program"), kernel->IsValid()))
				return;

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			if (!TestTrue(TEXT("Failed To Create Queue"), queue != nullptr))
				return;

			// Partial Range --------------------------------------------------
			size_t range = 2;

			Gpu::DispatchDescription dispatchDesc;
			dispatchDesc.Dim = 1;
			dispatchDesc.Global[0] = range;
			std::shared_ptr<Gpu::IEvent> event = queue->Dispatch(*kernel, dispatchDesc);
			if (!TestTrue(TEXT("Failed Partial Dispatch"), event != nullptr))
				return;

			event->Wait();

			std::vector<float> part_target_output = { 90, 6, 45, 19, 54 };

			std::vector<float> part_output_data(count, 0.0f);
			bufferInOut->Download(*queue, part_output_data.data(), range * sizeof(float));

			for (size_t i = 0; i < count; ++i)
			{
				if (i < range)
				{
					std::string msg = (std::to_string(part_target_output[i]) + " != " + std::to_string(part_output_data[i]));
					if (!TestTrue(FString(msg.c_str()), part_target_output[i] == part_output_data[i]))
						return;
				}
				else
				{
					if (!TestTrue("Values != 0", part_output_data[i] == 0))
						return;
				}
			}
			// ----------------------------------------------------------------


			// Full Range -----------------------------------------------------
			dispatchDesc.Global[0] = count;
			event = queue->Dispatch(*kernel, dispatchDesc);
			if (!TestTrue(TEXT("Failed Full Dispatch"), event != nullptr))
				return;

			event->Wait();

			std::vector<float> full_target_output = { 90, 6, 135, 57, 162 };

			std::vector<float> full_output_data(count, 0.0f);
			bufferInOut->Download(*queue, full_output_data.data(), count * sizeof(float));

			for (size_t i = 0; i < count; ++i)
			{
				std::string msg = (std::to_string(full_target_output[i]) + " != " + std::to_string(full_output_data[i]));
				if (!TestTrue(FString(msg.c_str()), full_target_output[i] == full_output_data[i]))
					return;
			}
			// ----------------------------------------------------------------
		});

		It("(3) Execution Synchronization", [this]()
		{
			//TODO:: Implement
		});
	});

	Describe("Images", [this]()
	{
		BeforeEach([this]()
		{
			Gpu::FactoryDesc desc;
			desc.PreferredBackend = Gpu::Backend::OpenCL;
			desc.bAllowFallback = false;

			mpCore = Gpu::Factory::Create(desc);
			mpDevice = mpCore->GetDevice(0);
			mpContext = mpCore->CreateContext(mpDevice);
		});

		AfterEach([this]()
		{
			mpCore.reset();
			mpDevice.reset();
			mpContext.reset();
		});

		It("(1) Image Creation", [this]()
		{
			Gpu::ImageDescription imageDesc;
			imageDesc.Type = Gpu::ImageType::Tex2D;
			imageDesc.Width = 256;
			imageDesc.Height = 256;
			imageDesc.Format = Gpu::PixelFormat::RGBA8;

			std::shared_ptr<Gpu::IImage> image = mpContext->CreateImage(imageDesc);
			TestNotNull(TEXT("Failed Image Creation!"), image.get());
		});
		
		It("(2) Image Read", [this]()
		{
			Gpu::ImageDescription imageDesc;
			imageDesc.Type = Gpu::ImageType::Tex2D;
			imageDesc.Width = 256;
			imageDesc.Height = 256;
			imageDesc.Format = Gpu::PixelFormat::RGBA8;

			std::shared_ptr<Gpu::IImage> image = mpContext->CreateImage(imageDesc);
			if (!TestNotNull(TEXT("Failed Image Creation!"), image.get()))
				return;

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			if (!TestTrue(TEXT("Failed To Create Queue"), queue != nullptr))
				return;

			Gpu::ImageRegion region;
			region.Width = image->GetWidth();
			region.Height = image->GetHeight();

			const size_t numBytes = image->GetBytesSize();
			std::vector<uint8_t> output_data(numBytes, 0);
			image->Download(*queue, output_data.data(), numBytes, region, {});

			const size_t numPixels = image->GetWidth() * image->GetHeight();
			for (size_t i = 0; i < numPixels; i += image->GetChannels())
			{
				const uint8_t r = output_data[i];
				const uint8_t g = output_data[i + 1];
				const uint8_t b = output_data[i + 2];
				const uint8_t a = output_data[i + 3];

				if (!TestTrue(TEXT("Pixel Isn't Empty Value!"), r == 0 && g == 0 && b == 0 && a == 0))
					return;
			}
		});

		It("(3) Image Fill", [this]()
		{
			Gpu::ImageDescription imageDesc;
			imageDesc.Type = Gpu::ImageType::Tex2D;
			imageDesc.Width = 256;
			imageDesc.Height = 256;
			imageDesc.Format = Gpu::PixelFormat::RGBA8;

			std::shared_ptr<Gpu::IImage> image = mpContext->CreateImage(imageDesc);
			if (!TestNotNull(TEXT("Failed Image Creation!"), image.get()))
				return;

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			if (!TestTrue(TEXT("Failed To Create Queue"), queue != nullptr))
				return;

			Gpu::ImageRegion region;
			region.Width = image->GetWidth();
			region.Height = image->GetHeight();

			const std::array<float, 4> fillColor = { 0, 0, 1.0, 1.0 };

			bool success = image->Fill(*queue, fillColor.data(), sizeof(fillColor), region);
			if (!TestTrue(TEXT("Failed To Fill Image"), success))
				return;

			const size_t numBytes = image->GetBytesSize();
			std::vector<uint8_t> output_data(numBytes, 0);

			success = image->Download(*queue, output_data.data(), numBytes, region, {});
			if (!TestTrue(TEXT("Failed To Download Image"), success))
				return;

			const std::array<uint8_t, 4> outPixelValue = { 0, 0, 255, 255 };

			const size_t numPixels = image->GetWidth() * image->GetHeight();
			for (size_t i = 0; i < numPixels; i += image->GetChannels())
			{
				const uint8_t r = output_data[i];
				const uint8_t g = output_data[i + 1];
				const uint8_t b = output_data[i + 2];
				const uint8_t a = output_data[i + 3];

				if (!TestTrue(TEXT("Pixel Isn't The Fill Color!"), r == outPixelValue[0] && g == outPixelValue[1] && b == outPixelValue[2] && a == outPixelValue[3]))
					return;
			}
		});

		It("(4) Image Program Write", [this]()
		{
			Gpu::ImageDescription imageDesc;
			imageDesc.Type = Gpu::ImageType::Tex2D;
			imageDesc.Width = 256;
			imageDesc.Height = 256;
			imageDesc.Format = Gpu::PixelFormat::RGBA8;

			std::shared_ptr<Gpu::IImage> image = mpContext->CreateImage(imageDesc);
			if (!TestNotNull(TEXT("Failed Image Creation!"), image.get()))
				return;

			std::shared_ptr<Gpu::IProgram> program = mpContext->CreateProgramFromSource("__kernel void write_red_img(read_write image2d_t output)\n" 
																						"{ const int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n"
																						"  write_imagef(output, coord, (float4)(1.0f, 0.0f, 0.0f, 1.0f)); }");
			if (!TestTrue(TEXT("Invalid Program"), program != nullptr))
				return;

			std::shared_ptr<Gpu::IKernel> kernel = program->CreateKernel("write_red_img");
			if (!TestTrue(TEXT("Kernel Invalid"), kernel != nullptr))
				return;

			std::shared_ptr<Gpu::IQueue> queue = mpContext->CreateQueue();
			if (!TestTrue(TEXT("Failed To Create Queue"), queue != nullptr))
				return;

			kernel->SetImageArg(0, *image);
			if (!TestTrue(TEXT("Failed to Set Image Argument"), kernel->IsValid()))
				return;

			Gpu::DispatchDescription dispatchDesc;
			dispatchDesc.Dim = 2;
			dispatchDesc.Global[0] = 256;
			dispatchDesc.Global[1] = 256;
			std::shared_ptr<Gpu::IEvent> event = queue->Dispatch(*kernel, dispatchDesc);
			if (!TestTrue(TEXT("Failed To Dispatch"), event != nullptr))
				return;

			queue->Finish();

			Gpu::ImageRegion region;
			region.Width = image->GetWidth();
			region.Height = image->GetHeight();

			const std::array<uint8_t, 4> fillColor = { 255, 0, 0, 255 };

			const size_t numBytes = image->GetBytesSize();
			std::vector<uint8_t> output_data(numBytes, 0);
			bool success = image->Download(*queue, output_data.data(), numBytes, region, {});
			if (!TestTrue(TEXT("Failed To Download Image"), success))
				return;

			const size_t numPixels = image->GetWidth() * image->GetHeight();
			const size_t numChannels = image->GetChannels();
			for (size_t i = 0; i < numPixels; i += numChannels)
			{
				const uint8_t r = output_data[i];
				const uint8_t g = output_data[i + 1];
				const uint8_t b = output_data[i + 2];
				const uint8_t a = output_data[i + 3];

				if (!TestTrue(TEXT("Pixel Isn't The Fill Color!"), r == fillColor[0] && g == fillColor[1] && b == fillColor[2] && a == fillColor[3]))
					return;
			}
		});
	});

	Describe("UE Interop", [this]()
	{
		BeforeEach([this]()
		{
			mpGPUContextObj = NewObject<UGPUContextObject>();
			if (mpGPUContextObj)
			{
				mpGPUContextObj->Initialize(EGPUBackend::OpenCL);
				mpGPUContextObj->CreateDefaultQueue();
			}
		});

		AfterEach([this]()
		{
			if (mpGPUContextObj)
			{
				mpGPUContextObj->ConditionalBeginDestroy();
				mpGPUContextObj = nullptr;
			}
		});

		It("(1) GPUContextObject", [this]()
		{
			if (!TestNotNull(TEXT("Failed to Create UGPUContextObject!"), mpGPUContextObj.Get()))
				return;

			TestTrue(TEXT("Failed To Create Context"), mpGPUContextObj->IsValidContext());

			if (!TestNotNull(TEXT("Default Queue is Null!"), mpGPUContextObj->GetDefaultQueue().get()))
				return;
		});

		It("(2) GPUProgramObject", [this]()
		{
			const FString programString("__kernel void test() { }");

			UGPUProgramObject* program = NewObject<UGPUProgramObject>();
			program->BuildFromSource(mpGPUContextObj, programString);

			if (!TestTrue(TEXT("Invalid program"), program->IsValidProgram()))
			{
				program->ConditionalBeginDestroy();
				return;
			}

			program->SetKernel("test");

			TestNotNull(TEXT("Created Kernel is Null!"), program->GetKernel().get());
			program->ConditionalBeginDestroy();
		});

		It("(3) GPUBufferObject", [this]()
		{
			TArray<float> testBufferData;
			testBufferData.Init(1.0f, 25);

			UGPUBufferObject* buffer = NewObject<UGPUBufferObject>();
			buffer->Initialize(mpGPUContextObj, testBufferData.Num() * sizeof(float));

			if (!TestTrue(TEXT("Invalid Buffer"), buffer->IsValidBuffer()))
			{
				buffer->ConditionalBeginDestroy();
				return;
			}

			buffer->UploadFloatArray(mpGPUContextObj, testBufferData);

			TArray<float> outputData;
			buffer->DownloadFloatArray(mpGPUContextObj, outputData);

			if (!TestTrue(TEXT("Buffer Data Mismatch"), testBufferData == outputData))
			{
				buffer->ConditionalBeginDestroy();
				return;
			}
			buffer->ConditionalBeginDestroy();
		});

		It("(4) GPUImageObject 2D", [this]()
		{
			const auto GPUImage2D = [&](EGpuPixelFormat format)
			{
				UGPUImageObject* image = NewObject<UGPUImageObject>();
				image->CreateImage2D(mpGPUContextObj,
									 mDefaultUTextureWidth,
									 mDefaultUTextureHeight,
									 format);

				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UGPUImageObject: %s"), *UEnum::GetValueAsString(format)), image))
				{
					return;
				}

				if (!TestTrue(FString::Printf(TEXT("Failed To Create Context for: %s"), *UEnum::GetValueAsString(format)), image->IsValidImage()))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				UTexture2D* utexture = image->CreateTexture2D(mpGPUContextObj, true);
				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UTexture2D for: %s"), *UEnum::GetValueAsString(format)), utexture))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				image->ConditionalBeginDestroy();
				utexture->ConditionalBeginDestroy();
			};
			
			GPUImage2D(EGpuPixelFormat::R8);
			GPUImage2D(EGpuPixelFormat::RG8);
			GPUImage2D(EGpuPixelFormat::RGBA8);

			GPUImage2D(EGpuPixelFormat::R32U);
			GPUImage2D(EGpuPixelFormat::RG32U);
			GPUImage2D(EGpuPixelFormat::RGBA32U);

			if (mpGPUContextObj->GetDevice()->GetCapabilities().bSupportHalfFloat)
			{
				GPUImage2D(EGpuPixelFormat::R16F);
				GPUImage2D(EGpuPixelFormat::RG16F);
				GPUImage2D(EGpuPixelFormat::RGBA16F);
			}

			GPUImage2D(EGpuPixelFormat::R32F);
			GPUImage2D(EGpuPixelFormat::RG32F);
			GPUImage2D(EGpuPixelFormat::RGBA32F);
		});

		It("(5) GPUImageObject 2D RenderTarget", [this]()
		{
			const auto GPUImage2DRT = [&](EGpuPixelFormat format)
			{
				UGPUImageObject* image = NewObject<UGPUImageObject>();
				image->CreateImage2D(mpGPUContextObj,
									 mDefaultUTextureWidth,
									 mDefaultUTextureHeight,
									 format);

				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UGPUImageObject: %s"), *UEnum::GetValueAsString(format)), image))
				{
					return;
				}

				if (!TestTrue(FString::Printf(TEXT("Failed To Create Context for: %s"), *UEnum::GetValueAsString(format)), image->IsValidImage()))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				UTextureRenderTarget2D* utexture = image->CreateAndWriteRenderTarget2D(mpGPUContextObj);
				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UTextureRenderTarget2D for: %s"), *UEnum::GetValueAsString(format)), utexture))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				image->ConditionalBeginDestroy();
				utexture->ConditionalBeginDestroy();
			};
			
			GPUImage2DRT(EGpuPixelFormat::R8);
			GPUImage2DRT(EGpuPixelFormat::RG8);
			GPUImage2DRT(EGpuPixelFormat::RGBA8);

			if (mpGPUContextObj->GetDevice()->GetCapabilities().bSupportHalfFloat)
			{
				GPUImage2DRT(EGpuPixelFormat::R16F);
				GPUImage2DRT(EGpuPixelFormat::RG16F);
				GPUImage2DRT(EGpuPixelFormat::RGBA16F);
			}

			GPUImage2DRT(EGpuPixelFormat::R32F);
			GPUImage2DRT(EGpuPixelFormat::RG32F);
			GPUImage2DRT(EGpuPixelFormat::RGBA32F);
		});

		It("(6) GPUImageObject 2D Array", [this]()
		{
			const auto GPUImage2DArray = [&](EGpuPixelFormat format)
			{
				UGPUImageObject* image = NewObject<UGPUImageObject>();
				image->CreateImage2DArray(mpGPUContextObj,
										  mDefaultUTextureWidth,
										  mDefaultUTextureHeight,
										  mDefaultUTextureArraySlices,
										  format);

				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UGPUImageObject: %s"), *UEnum::GetValueAsString(format)), image))
				{
					return;
				}

				if (!TestTrue(FString::Printf(TEXT("Failed To Create Context for: %s"), *UEnum::GetValueAsString(format)), image->IsValidImage()))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				UTexture2DArray* utexture = image->CreateTexture2DArray(mpGPUContextObj, true);
				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UTexture2DArray for: %s"), *UEnum::GetValueAsString(format)), utexture))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				image->ConditionalBeginDestroy();
				utexture->ConditionalBeginDestroy();
			};
			
			GPUImage2DArray(EGpuPixelFormat::R8);
			GPUImage2DArray(EGpuPixelFormat::RG8);
			GPUImage2DArray(EGpuPixelFormat::RGBA8);

			GPUImage2DArray(EGpuPixelFormat::R32U);
			GPUImage2DArray(EGpuPixelFormat::RG32U);
			GPUImage2DArray(EGpuPixelFormat::RGBA32U);

			if (mpGPUContextObj->GetDevice()->GetCapabilities().bSupportHalfFloat)
			{
				GPUImage2DArray(EGpuPixelFormat::R16F);
				GPUImage2DArray(EGpuPixelFormat::RG16F);
				GPUImage2DArray(EGpuPixelFormat::RGBA16F);
			}

			GPUImage2DArray(EGpuPixelFormat::R32F);
			GPUImage2DArray(EGpuPixelFormat::RG32F);
			GPUImage2DArray(EGpuPixelFormat::RGBA32F);
		});

		It("(7) GPUImageObject 3D", [this]()
		{
			const auto GPUImage3D = [&](EGpuPixelFormat format)
			{
				UGPUImageObject* image = NewObject<UGPUImageObject>();
				image->CreateImage3D(mpGPUContextObj,
								     mDefaultUTextureWidth,
								     mDefaultUTextureHeight,
								     mDefaultUTextureDepth,
								     format);

				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UGPUImageObject: %s"), *UEnum::GetValueAsString(format)), image))
				{
					return;
				}

				if (!TestTrue(FString::Printf(TEXT("Failed To Create Context for: %s"), *UEnum::GetValueAsString(format)), image->IsValidImage()))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				UVolumeTexture* utexture = image->CreateVolumeTexture(mpGPUContextObj, true);
				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UVolumeTexture for: %s"), *UEnum::GetValueAsString(format)), utexture))
				{
					image->ConditionalBeginDestroy();
					return;
				}

				image->ConditionalBeginDestroy();
				utexture->ConditionalBeginDestroy();
			};
			
			GPUImage3D(EGpuPixelFormat::R8);
			GPUImage3D(EGpuPixelFormat::RG8);
			GPUImage3D(EGpuPixelFormat::RGBA8);
		});

		It("(8) GPUImageObject RenderTarget Read/Write", [this]()
		{
			const EGpuPixelFormat format = EGpuPixelFormat::RGBA8;

			UGPUImageObject* image = NewObject<UGPUImageObject>();
			image->CreateImage2D(mpGPUContextObj,
								 mDefaultUTextureWidth,
								 mDefaultUTextureHeight,
								 format);

			if (!TestNotNull(FString::Printf(TEXT("Failed to Create UGPUImageObject: %s"), *UEnum::GetValueAsString(format)), image))
			{
				return;
			}

			if (!TestTrue(FString::Printf(TEXT("Failed To Create Context for: %s"), *UEnum::GetValueAsString(format)), image->IsValidImage()))
			{
				image->ConditionalBeginDestroy();
				return;
			}

			const FString programString("__kernel void write_red_img(read_write image2d_t output)\n" 
										"{ const int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n"
										"  write_imagef(output, coord, (float4)(0.0f, 1.0f, 0.0f, 1.0f)); }");

			UGPUProgramObject* program = NewObject<UGPUProgramObject>();
			program->BuildFromSource(mpGPUContextObj, programString);

			program->SetKernel("write_red_img");

			program->SetImageArg(0, image);
			if (!TestTrue(TEXT("Failed to Set Image Argument"), program->GetKernel()->IsValid()))
				return;

			Gpu::DispatchDescription dispatchDesc;
			dispatchDesc.Dim = 2;
			dispatchDesc.Global[0] = mDefaultUTextureWidth;
			dispatchDesc.Global[1] = mDefaultUTextureHeight;
			std::shared_ptr<Gpu::IEvent> event = mpGPUContextObj->GetDefaultQueue()->Dispatch(*program->GetKernel(), dispatchDesc);
			if (!TestTrue(TEXT("Failed To Dispatch"), event != nullptr))
				return;

			mpGPUContextObj->GetDefaultQueue()->Finish();

			UTextureRenderTarget2D* utexture = image->CreateAndWriteRenderTarget2D(mpGPUContextObj);

			TUniquePtr<FTestUWorld> tempWorld = MakeUnique<FTestUWorld>();
			FColor rt_color = UKismetRenderingLibrary::ReadRenderTargetPixel(tempWorld->GetWorld(), utexture, 0, 0);

			TestTrue(TEXT("Incorrect Color In Render Target2D!"), rt_color == FColor::Green);

			utexture->ConditionalBeginDestroy();
			tempWorld.Reset();
		});

		LatentIt("(9) GPUImageObject 2D Async Update", EAsyncExecution::ThreadPool, FTimespan(0, 0, 20), [this](const FDoneDelegate& Done)
		{
			UGPUImageObject* image = nullptr;
			UTexture2D* utexture = nullptr;
			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&image, &utexture, this]()
			{
				const EGpuPixelFormat format = EGpuPixelFormat::RGBA8;

				image = NewObject<UGPUImageObject>();
				image->AddToRoot();
				image->CreateImage2D(mpGPUContextObj,
									 mDefaultUTextureWidth,
									 mDefaultUTextureHeight,
									 format);

				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UGPUImageObject: %s"), *UEnum::GetValueAsString(format)), image))
				{
					return;
				}

				if (!TestTrue(FString::Printf(TEXT("Failed To Create Context for: %s"), *UEnum::GetValueAsString(format)), image->IsValidImage()))
				{
					image->RemoveFromRoot();
					image->ConditionalBeginDestroy();
					return;
				}

				utexture = image->CreateTexture2D(mpGPUContextObj, true);
				utexture->AddToRoot();
				if (!TestNotNull(FString::Printf(TEXT("Failed to Create UTexture2D for: %s"), *UEnum::GetValueAsString(format)), utexture))
				{
					image->RemoveFromRoot();
					image->ConditionalBeginDestroy();
					return;
				}
			}, TStatId(), nullptr, ENamedThreads::GameThread);
			Task->Wait();

			bool fillSuccess = image->FillColor(mpGPUContextObj, FColor::Blue);
			if (!TestTrue(TEXT("Failed to Fill Color"), fillSuccess))
			{
				image->RemoveFromRoot();
				image->ConditionalBeginDestroy();
				return;
			}

			image->UpdateTexture2DAsync(mpGPUContextObj, utexture, [image, utexture, Done, this](bool success)
			{
				check(IsInGameThread());

				if (!TestTrue(TEXT("Failed Async Update"), success))
				{
					Done.Execute();
					return;
				}

				// TODO:: Add Pixel Reading Check.

				image->RemoveFromRoot();
				image->ConditionalBeginDestroy();
				utexture->RemoveFromRoot();
				utexture->ConditionalBeginDestroy();
				Done.Execute();
			}, ENamedThreads::GameThread);
		});
	});
}
