// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsGameState.h"

ASteamFpsGameState::ASteamFpsGameState()
    : m_remainingTime(0)
    , m_numTeams(0)
    , m_timerPaused(false)
{
}

/** Called when the state transitions to WaitingToStart */
void ASteamFpsGameState::HandleMatchIsWaitingToStart()
{
    V_TRACE_MARKER();
    Super::HandleMatchIsWaitingToStart();
}

/** Called when the state transitions to InProgress */
void ASteamFpsGameState::HandleMatchHasStarted()
{
    V_TRACE_MARKER();
    Super::HandleMatchHasStarted();
}

/** Called when the map transitions to WaitingPostMatch */
void ASteamFpsGameState::HandleMatchHasEnded()
{
    V_TRACE_MARKER();
    Super::HandleMatchHasEnded();
}

/** Called when the match transitions to LeavingMap */
void ASteamFpsGameState::HandleLeavingMap()
{
    V_TRACE_MARKER();
    Super::HandleLeavingMap();
}

void ASteamFpsGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASteamFpsGameState, m_numTeams);
    DOREPLIFETIME(ASteamFpsGameState, m_remainingTime);
    DOREPLIFETIME(ASteamFpsGameState, m_timerPaused);
    DOREPLIFETIME(ASteamFpsGameState, m_teamScores);
}

