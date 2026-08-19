#pragma once

#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"

#include "EngineUtils.h"

/// <summary>
/// Test world class for unit testing. It creates a new world and destroys it when the test is done. It also provides a getter for the world pointer.
/// </summary>
class FTestUWorld
{
public:
    /// <summary>
    /// Constructor initializing a FTestUWorld.
    /// </summary>
    /// <param name="URL">The FURL</param>
    FTestUWorld(const FURL& URL = FURL());

    /// <summary>
    /// Default destructor.
    /// </summary>
    ~FTestUWorld();
public:
    /// <summary>
	/// Retrieves the UWorld pointer.
    /// </summary>
    /// <returns>The UWorld pointer</returns>
    UWorld* GetWorld() const { return WeakWorld.Get(); }
private:
    TWeakObjectPtr<UWorld> WeakWorld;
};