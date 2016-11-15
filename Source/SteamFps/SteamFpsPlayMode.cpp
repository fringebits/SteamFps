// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsGameInstance.h"
#include "SteamFpsPlayMode.h"
#include "Gameplay/SteamFpsHUD.h"

ASteamFpsPlayMode::ASteamFpsPlayMode()
{
    V_TRACE_MARKER();

    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/FirstPersonCharacter"));
    DefaultPawnClass = PlayerPawnClassFinder.Class;

    // use our custom HUD class
    HUDClass = ASteamFpsHUD::StaticClass();
}

void ASteamFpsPlayMode::BeginPlay()
{
    Super::BeginPlay();
    V_TRACE_MARKER();

    ExecuteOnceAfter("CallEndMatch", 15.0f, [=] () { this->EndMatch(); });
    ExecuteOnPeriod("ReportGameState", 5.0f, [=]() { 
        auto ms = GetMatchState();
        V_LOG(GeneralLog, "GameState=%s", *(ms.ToString()));
    });
}

void ASteamFpsPlayMode::Tick(float dT)
{
    Super::Tick(dT);
}

void ASteamFpsPlayMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();
    V_TRACE_MARKER();
}

void ASteamFpsPlayMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    V_TRACE_MARKER();

    auto gi = USteamFpsGameInstance::GetInstance();
    V_CHECK_VALID(gi);
    gi->OpenLevel("LobbyMap", true);
}

void ASteamFpsPlayMode::HandleLeavingMap()
{
    Super::HandleLeavingMap();
    V_TRACE_MARKER();
}