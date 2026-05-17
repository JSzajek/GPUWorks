#include "Profiler/GPUProfilerManager.h"
#include "Profiler/GPUStats.h"

#include "Stats/Stats.h"

#include "GPUWorksLog.h"

std::mutex FGPUProfilerManager::Mutex;
TArray<TSharedPtr<Gpu::IProfilerBackend>> FGPUProfilerManager::Backends;
TArray<FGPUProfilerManager::FActiveProfile> FGPUProfilerManager::ActiveProfiles;
TArray<Gpu::KernelProfile> FGPUProfilerManager::CompletedProfiles;

void FGPUProfilerManager::RegisterBackend(TSharedPtr<Gpu::IProfilerBackend> Backend)
{
    if (!Backend)
    {
        return;
    }

    std::scoped_lock Lock(Mutex);
    for (const TSharedPtr<Gpu::IProfilerBackend>& Existing : Backends)
    {
        if (Existing && Existing->GetBackend() == Backend->GetBackend())
        {
            return;
        }
    }

    Backends.Add(Backend);

	// Query hardware metrics for stats display
    Gpu::HardwareMetrics Metrics = Backend->QueryHardwareMetrics();
    SET_DWORD_STAT(STAT_GGPU_TotalComputeUnits, Metrics.mComputeUnitCount);
    SET_DWORD_STAT(STAT_GGPU_TotalWorkgroups, Metrics.mMaxWorkGroupSize);
    SET_MEMORY_STAT(STAT_GGPU_HardwareGlobalMemory, Metrics.mGlobalMemoryBytes);
	SET_MEMORY_STAT(STAT_GGPU_HardwareLocalMemory, Metrics.mLocalMemoryBytes);
}

TSharedPtr<Gpu::IProfilerBackend> FGPUProfilerManager::FindBackend(Gpu::Backend Backend)
{
    for (const TSharedPtr<Gpu::IProfilerBackend>& Candidate : Backends)
    {
        if (Candidate && Candidate->GetBackend() == Backend)
        {
            return Candidate;
        }
    }
    return nullptr;
}

void FGPUProfilerManager::EnqueueProfiledKernel(const Gpu::ProfiledKernelHandle& InHandle,
                                                const Gpu::KernelDispatchInfo& DispatchInfo)
{
    std::scoped_lock Lock(Mutex);

    TSharedPtr<Gpu::IProfilerBackend> Backend = FindBackend(InHandle.Backend);
    if (!Backend)
    {
        return;
    }

    Gpu::ProfiledKernelHandle Handle = InHandle;
    if (!Backend->RetainProfiledHandle(Handle))
    {
        return;
    }

    FActiveProfile Active;
    Active.Handle = Handle;
    Active.Profile.Dispatch = DispatchInfo;

    Backend->QueryKernelStaticInfo(Handle, Active.Profile.StaticInfo);

    ActiveProfiles.Add(MoveTemp(Active));
}

void FGPUProfilerManager::Tick(float DeltaTime)
{
    PollEvents();
    UpdateStats();
}

TStatId FGPUProfilerManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(FGPUProfilerManager, STATGROUP_Tickables);
}

void FGPUProfilerManager::PollEvents()
{
    std::scoped_lock Lock(Mutex);

    for (int32 i = ActiveProfiles.Num() - 1; i >= 0; --i)
    {
        FActiveProfile& Active = ActiveProfiles[i];

        TSharedPtr<Gpu::IProfilerBackend> Backend = FindBackend(Active.Handle.Backend);
        if (!Backend)
        {
            ActiveProfiles.RemoveAtSwap(i);
            continue;
        }

        if (!Backend->IsComplete(Active.Handle))
        {
            continue;
        }

        uint64 StartNs = 0;
        uint64 EndNs = 0;

        Active.Profile.bComplete = true;
        Active.Profile.bValidTiming = Backend->QueryTiming(Active.Handle, StartNs, EndNs);
        Active.Profile.StartTimeNs = StartNs;
        Active.Profile.EndTimeNs = EndNs;

        Backend->ReleaseProfiledHandle(Active.Handle);

        CompletedProfiles.Add(MoveTemp(Active.Profile));
        ActiveProfiles.RemoveAtSwap(i);
    }
}

void FGPUProfilerManager::UpdateStats()
{
    std::scoped_lock Lock(Mutex);

    SET_DWORD_STAT(STAT_GGPU_ActiveKernels, ActiveProfiles.Num());

    float TotalKernelTimeMs = 0.0f;
    uint64_t TotalWorkGroups = 0;
    uint64_t TotalPrivateMemSize = 0;
    uint64_t TotalLocalMemSize = 0;
    uint64_t TotalPreferredWorkGroupMultiple = 0;
    std::array<uint64_t, 3> TotalCompiledWorkGroupSize = { 0 };


    for (const Gpu::KernelProfile& Profile : CompletedProfiles)
    {
        TotalKernelTimeMs += Profile.GetDurationMs();
        TotalWorkGroups += Profile.GetWorkGroupCount();
        TotalPrivateMemSize += Profile.StaticInfo.PrivateMemoryBytes;
        TotalLocalMemSize += Profile.StaticInfo.LocalMemoryBytes;
        TotalPreferredWorkGroupMultiple += Profile.StaticInfo.PreferredWorkGroupMultiple;
        TotalCompiledWorkGroupSize[0] += Profile.StaticInfo.CompiledWorkGroupSize[0];
        TotalCompiledWorkGroupSize[1] += Profile.StaticInfo.CompiledWorkGroupSize[1];
        TotalCompiledWorkGroupSize[2] += Profile.StaticInfo.CompiledWorkGroupSize[2];
    }

    SET_FLOAT_STAT(STAT_GGPU_KernelTimeMs, TotalKernelTimeMs);
    SET_DWORD_STAT(STAT_GGPU_CompletedKernels, CompletedProfiles.Num());
    SET_DWORD_STAT(STAT_GGPU_TotalWorkgroups, static_cast<int32>(TotalWorkGroups));
    SET_DWORD_STAT(STAT_GGPU_PreferredWorkgroupsMultiple, TotalPreferredWorkGroupMultiple);
    SET_DWORD_STAT(STAT_GGPU_CompiledWorkGroupsize_Dim1, TotalCompiledWorkGroupSize[0]);
    SET_DWORD_STAT(STAT_GGPU_CompiledWorkGroupsize_Dim2, TotalCompiledWorkGroupSize[1]);
    SET_DWORD_STAT(STAT_GGPU_CompiledWorkGroupsize_Dim3, TotalCompiledWorkGroupSize[2]);
    SET_MEMORY_STAT(STAT_GGPU_ActivePrivateMemory, TotalPrivateMemSize);
    SET_MEMORY_STAT(STAT_GGPU_ActiveLocalMemory, TotalLocalMemSize);

    CompletedProfiles.Reset();
}



#if 0
FGPUHardwareMetrics FGPUProfilerManager::HardwareMetrics = {};

TArray<FGPUKernelProfile> FGPUProfilerManager::ActiveKernels = {};
TArray<FGPUKernelProfile> FGPUProfilerManager::CompletedKernels = {};

std::mutex mProfileMutex = {};

FGPUProfilerManager::FGPUProfilerManager()
{
#if 0
	// Profile default hardware statistics
	OpenCL::Device device;

	clGetDeviceInfo(device.Get(), CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(HardwareMetrics.MaxComputeUnits), &HardwareMetrics.MaxComputeUnits, nullptr);
	clGetDeviceInfo(device.Get(), CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(HardwareMetrics.MaxWorkGroupSize), &HardwareMetrics.MaxWorkGroupSize, nullptr);
	clGetDeviceInfo(device.Get(), CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(HardwareMetrics.GlobalMemSize), &HardwareMetrics.GlobalMemSize, nullptr);
	clGetDeviceInfo(device.Get(), CL_DEVICE_LOCAL_MEM_SIZE, sizeof(HardwareMetrics.LocalMemSize), &HardwareMetrics.LocalMemSize, nullptr);

	SET_DWORD_STAT(STAT_OpenCL_TotalComputeUnits, HardwareMetrics.MaxComputeUnits);
	SET_DWORD_STAT(STAT_OpenCL_TotalWorkgroups, HardwareMetrics.MaxWorkGroupSize);

	SET_MEMORY_STAT(STAT_OpenCL_HardwareGlobalMemory, HardwareMetrics.GlobalMemSize);
	SET_MEMORY_STAT(STAT_OpenCL_HardwareLocalMemory, HardwareMetrics.LocalMemSize);
#endif
}

FGPUProfilerManager::~FGPUProfilerManager()
{
}


void FGPUProfilerManager::Tick(float DeltaTime)
{
	PollEvents();
	UpdateStats();
}

TStatId FGPUProfilerManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FGPUProfilerManager, STATGROUP_Tickables);
}

void FGPUProfilerManager::EnqueueProfiledKernel(&Gpu::IQueue queue,
												&Gpu::IKernel kernel,
												&Gpu::IEvent event,
												size_t work_dim,
												const size_t* global_work_size,
												const size_t* local_work_size)
{
	const std::scoped_lock lock(mProfileMutex);

	FKernelProfile Profile;
	Profile.mName = kernel.GetName();
	Profile.mEvent = event.Get();
	
	OpenCL::DevicePtr device_ptr = queue.GetDevicePtr();
	if (!device_ptr)
	{
		UE_LOG(LogGPUWorks, Warning, TEXT("Failed To Enqueue Kernel: %s"), *FString(kernel.GetName().c_str()));
		return;
	}

	clGetKernelWorkGroupInfo(kernel, *device_ptr, CL_KERNEL_WORK_GROUP_SIZE, sizeof(Profile.KernelWorkGroupSize), &Profile.KernelWorkGroupSize, nullptr);
	clGetKernelWorkGroupInfo(kernel, *device_ptr, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(Profile.PreferredWorkGroupMultiple), &Profile.PreferredWorkGroupMultiple, nullptr);
	clGetKernelWorkGroupInfo(kernel, *device_ptr, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(Profile.CompiledWorkGroupSize), &Profile.CompiledWorkGroupSize, nullptr);
	clGetKernelWorkGroupInfo(kernel, *device_ptr, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(Profile.PrivateMemSize), &Profile.PrivateMemSize, nullptr);
	clGetKernelWorkGroupInfo(kernel, *device_ptr, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(Profile.LocalMemSizeUsed), &Profile.LocalMemSizeUsed, nullptr);

	ActiveKernels.Add(Profile);
}

void FGPUProfilerManager::PollEvents()
{
	const std::scoped_lock lock(mProfileMutex);

	for (int32 i = ActiveKernels.Num() - 1; i >= 0; --i)
	{
		cl_int eventStatus;
		clGetEventInfo(ActiveKernels[i].mEvent, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(eventStatus), &eventStatus, nullptr);

		if (eventStatus == CL_COMPLETE)
		{
			clGetEventProfilingInfo(ActiveKernels[i].mEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ActiveKernels[i].mStartTimeNs, nullptr);
			clGetEventProfilingInfo(ActiveKernels[i].mEvent, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ActiveKernels[i].mEndTimeNs, nullptr);

			clReleaseEvent(ActiveKernels[i].mEvent);
			CompletedKernels.Add(ActiveKernels[i]);
			ActiveKernels.RemoveAt(i);
		}
	}
}

void FGPUProfilerManager::UpdateStats()
{
	const std::scoped_lock lock(mProfileMutex);

	SET_DWORD_STAT(STAT_OpenCL_ActiveKernels, ActiveKernels.Num());

	if (CompletedKernels.Num() > 0)
	{
		float TotalTime_ms = 0.0f;
		int32 TotalGroups = 0;

		for (const FKernelProfile& Profile : CompletedKernels)
		{
			TotalTime_ms += Profile.GetDurationMs();
			TotalGroups += Profile.GetWorkGroupCount();
		}

		SET_FLOAT_STAT(STAT_OpenCL_KernelTime, TotalTime_ms);

		CompletedKernels.Empty();
	}
	else
	{
		SET_FLOAT_STAT(STAT_OpenCL_KernelTime, 0.0f);
	}
}


#endif