#pragma once

#include "Tickable.h"
#include "GpuProfilerTypes.h"
#include "IGpuProfilerBackend.h"

#include <mutex>

/// <summary>
/// GPU profiler manager responsible for tracking active GPU profiles,
/// polling registered profiler backends for completed profiles, 
/// and pushing stats to Unreal's stats system for display in the editor and profiler.
/// </summary>
class FGPUProfilerManager : public FTickableGameObject
{
public:
    /// <summary>
	/// Overriden tick function that polls registered profiler backends for completed profiles.
    /// </summary>
    /// <param name="DeltaTime">The time elapsed since the last tick</param>
    virtual void Tick(float DeltaTime) override;

    /// <summary>
    /// Whether the tickable game object should be ticked when the game is paused.
    /// </summary>
    /// <returns>True if the tickable game object should be ticked when the game is paused</returns>
    virtual bool IsTickableWhenPaused() const override { return true; }

    /// <summary>
    /// Whether the tickable game object should be ticked in the editor.
    /// </summary>
    /// <returns>True if the tickable game object should be ticked in the editor</returns>
    virtual bool IsTickableInEditor() const override { return true; }

    /// <summary>
    /// Whether the tickable game object should be ticked at all.
    /// </summary>
    /// <returns>True if the tickable game object should be ticked at all</returns>
    virtual bool IsTickable() const override { return true; }

    /// <summary>
	/// Retrieves the stat ID for this tickable game object, used for grouping stats in the editor and profiler.
    /// </summary>
    /// <returns>The stat ID</returns>
    virtual TStatId GetStatId() const override;
public:
    /// <summary>
	/// Registers a GPU profiler backend with the manager.
    /// </summary>
    /// <param name="backend">The GPU backend</param>
    static void RegisterBackend(TSharedPtr<Gpu::IProfilerBackend> backend);

    /// <summary>
	/// Enqueues the specified profiled kernel for tracking and stats display.
    /// </summary>
    /// <param name="handle">The profiled kernel handle</param>
    /// <param name="info">The kernel dispatch information</param>
    static void EnqueueProfiledKernel(const Gpu::ProfiledKernelHandle& handle,
                                      const Gpu::KernelDispatchInfo& info);
private:
    /// <summary>
	/// Active profile that is currently being tracked.
    /// </summary>
    struct FActiveProfile
    {
        Gpu::ProfiledKernelHandle Handle;
        Gpu::KernelProfile Profile;
    };

    /// <summary>
	/// Returns the registered profiler backend for the specified GPU backend, or nullptr if not found.
    /// </summary>
    /// <param name="backend">The GPU backend</param>
    /// <returns>The registered profiler backend, or nullptr if not found</returns>
    static TSharedPtr<Gpu::IProfilerBackend> FindBackend(Gpu::Backend backend);

    /// <summary>
	/// Poll the registered profiler backends for completed profiles, move them to the completed list and update stats.
    /// </summary>
    void PollEvents();

    /// <summary>
	/// Update stats based on completed profiles and push to Unreal's stats system for display in the editor and profiler.
    /// </summary>
    void UpdateStats();
private:
    static std::mutex Mutex;

    static TArray<TSharedPtr<Gpu::IProfilerBackend>> Backends;
    static TArray<FActiveProfile> ActiveProfiles;
    static TArray<Gpu::KernelProfile> CompletedProfiles;
};