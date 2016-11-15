// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include "STeamFpsGameMode.h"
#include "SteamFpsLobbyMode.generated.h"

UCLASS()
class ASteamFpsLobbyMode : public ASteamFpsGameMode
{
	GENERATED_BODY()

public:
    ASteamFpsLobbyMode();

    virtual void BeginPlay() override;

private:
    void CreateOnlineSession();
};