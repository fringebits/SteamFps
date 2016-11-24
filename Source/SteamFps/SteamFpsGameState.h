// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "SteamFpsGameState.generated.h"

UCLASS()
class STEAMFPS_API ASteamFpsGameState : public AGameState
{
	GENERATED_BODY()

protected:
    /** Called when the state transitions to WaitingToStart */
    virtual void HandleMatchIsWaitingToStart() override;

    /** Called when the state transitions to InProgress */
    virtual void HandleMatchHasStarted() override;

    /** Called when the map transitions to WaitingPostMatch */
    virtual void HandleMatchHasEnded() override;

    /** Called when the match transitions to LeavingMap */
    virtual void HandleLeavingMap() override;
};
