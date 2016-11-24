// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsGameState.h"

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