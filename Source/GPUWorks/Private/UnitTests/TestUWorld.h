#pragma once

#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"

#include "EngineUtils.h"

class FTestUWorld
{
public:
    explicit FTestUWorld(const FURL& URL = FURL());
    ~FTestUWorld();
public:
    UWorld* GetWorld() const { return WeakWorld.Get(); }
private:
    TWeakObjectPtr<UWorld> WeakWorld;
};