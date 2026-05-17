#include "TestUWorld.h"

#include "Engine/Engine.h"

FTestUWorld::FTestUWorld(const FURL& URL)

{
    if (GEngine)
    {
        static uint32 WorldCounter = 0;
        const FString WorldName = FString::Printf(TEXT("TestUWorld_%d"), WorldCounter++);

        if (UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, *WorldName, GetTransientPackage()))
        {
            FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
            WorldContext.SetCurrentWorld(World);

            World->InitializeActorsForPlay(URL);
            if (IsValid(World->GetWorldSettings()))
            {
                // Need to do this manually since world doesn't have a game mode
                World->GetWorldSettings()->NotifyBeginPlay();
                World->GetWorldSettings()->NotifyMatchStarted();
            }
            World->BeginPlay();

            WeakWorld = MakeWeakObjectPtr(World);
        }
    }
}

FTestUWorld::~FTestUWorld()
{
    UWorld* World = WeakWorld.Get();
    if (World && GEngine)
    {
        World->EndPlay(EEndPlayReason::Type::RemovedFromWorld);

        GEngine->DestroyWorldContext(World);
        World->DestroyWorld(false);
    }
}