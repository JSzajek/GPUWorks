// Copyright Epic Games, Inc. All Rights Reserved.

#include "GPUWorks.h"

#include "Interfaces/IPluginManager.h"

#include "Profiler/GPUProfilerManager.h"

#include "Backends/OpenCL/OpenCLProfilerBackend.h"

#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FGPUWorksModule"

void FGPUWorksModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("GPUWorks"))->GetBaseDir(), TEXT("/Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/GPUShaders"), PluginShaderDir);

	mpGPUProfilerManager = MakeUnique<FGPUProfilerManager>();

	FGPUProfilerManager::RegisterBackend(MakeShared<Gpu::OpenCL::ProfilerBackend>());
}

void FGPUWorksModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	mpGPUProfilerManager.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGPUWorksModule, GPUWorks)