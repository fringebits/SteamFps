// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include "SteamFpsGameMode.h"
#include "SteamFpsPlayMode.generated.h"

UCLASS()
class ASteamFpsPlayMode : public ASteamFpsGameMode
{
	GENERATED_BODY()

public:
    ASteamFpsPlayMode();

    virtual void BeginPlay() override;
    virtual void Tick(float dT) override;

protected:
    virtual void HandleMatchHasStarted() override;
    virtual void HandleMatchHasEnded() override;
    virtual void HandleLeavingMap() override;
};