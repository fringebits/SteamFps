// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "SteamFpsGameState.generated.h"

UCLASS()
class STEAMFPS_API ASteamFpsGameState : public AGameState
{
	GENERATED_BODY()
public:
    ASteamFpsGameState();

protected:
    /** Called when the state transitions to WaitingToStart */
    virtual void HandleMatchIsWaitingToStart() override;

    /** Called when the state transitions to InProgress */
    virtual void HandleMatchHasStarted() override;

    /** Called when the map transitions to WaitingPostMatch */
    virtual void HandleMatchHasEnded() override;

    /** Called when the match transitions to LeavingMap */
    virtual void HandleLeavingMap() override;

public: // BUGBUG: these should *not* be public!!!

    /** number of teams in current game (doesn't deprecate when no players are left in a team) */
    UPROPERTY(Transient, Replicated)
    int32 m_numTeams;

    /** accumulated score per team */
    UPROPERTY(Transient, Replicated)
    TArray<int32> m_teamScores;

    /** time left for warmup / match */
    UPROPERTY(Transient, Replicated)
    int32 m_remainingTime;

    /** is timer paused? */
    UPROPERTY(Transient, Replicated)
    bool m_timerPaused;
};
