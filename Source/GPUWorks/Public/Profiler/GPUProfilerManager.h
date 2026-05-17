#pragma once

#include "Tickable.h"
#include "GpuProfilerTypes.h"
#include "IGpuProfilerBackend.h"

#include <mutex>

class FGPUProfilerManager : public FTickableGameObject
{
public:
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickableWhenPaused() const override { return true; }
    virtual bool IsTickableInEditor() const override { return true; }
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;
public:
    static void RegisterBackend(TSharedPtr<Gpu::IProfilerBackend> Backend);

    static void EnqueueProfiledKernel(const Gpu::ProfiledKernelHandle& Handle,
                                      const Gpu::KernelDispatchInfo& DispatchInfo);
private:
    struct FActiveProfile
    {
        Gpu::ProfiledKernelHandle Handle;
        Gpu::KernelProfile Profile;
    };

    static TSharedPtr<Gpu::IProfilerBackend> FindBackend(Gpu::Backend Backend);

    void PollEvents();
    void UpdateStats();

private:
    static std::mutex Mutex;

    static TArray<TSharedPtr<Gpu::IProfilerBackend>> Backends;
    static TArray<FActiveProfile> ActiveProfiles;
    static TArray<Gpu::KernelProfile> CompletedProfiles;
};