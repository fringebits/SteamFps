// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsLobbyMode.h"
#include "SteamFpsGameSession.h"

ASteamFpsLobbyMode::ASteamFpsLobbyMode()
{
    V_TRACE_MARKER();
}

void ASteamFpsLobbyMode::BeginPlay()
{
    Super::BeginPlay();

    ExecuteOnceAfter("CreateOnlineSession", 5.0f, [=] () { this->CreateOnlineSession(); });
}

void ASteamFpsLobbyMode::CreateOnlineSession()
{
    V_TRACE();

    // We've been in the lobby long enough -- create a game server session.
    auto gs = Cast<ASteamFpsGameSession>(GameSession);
    V_CHECK_VALID(gs);
    gs->CreateOnlineSession();
}