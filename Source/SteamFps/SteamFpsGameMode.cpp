// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsGameInstance.h"
#include "SteamFpsGameMode.h"
#include "SteamFpsGameSession.h"
#include "Gameplay/SteamFpsHUD.h"
#include "Gameplay/SteamFpsCharacter.h"

ASteamFpsGameMode::ASteamFpsGameMode()
{
    V_TRACE_MARKER();

    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/FirstPersonCharacter"));
    DefaultPawnClass = PlayerPawnClassFinder.Class;

    // use our custom HUD class
    HUDClass = ASteamFpsHUD::StaticClass();
}

void ASteamFpsGameMode::BeginPlay()
{
    V_TRACE_MARKER();
    Super::BeginPlay();

    //ExecuteOnceAfter("CallEndMatch", 15.0f, [=]() { this->EndMatch(); });

    ExecuteOnPeriod("ReportGameState", 5.0f, [=]() {
        auto ms = GetMatchState();
        V_LOG(GeneralLog, "GameState=%s", *(ms.ToString()));
    });
}

void ASteamFpsGameMode::Tick(float dT)
{
    Super::Tick(dT);

    TickTimers(dT);
}

void ASteamFpsGameMode::InitGame(const FString& mapName, const FString& options, FString& errorMessage)
{
    V_TRACE_MARKER();
    Super::InitGame(mapName, options, errorMessage);
}

void ASteamFpsGameMode::PreLogin(const FString& options, const FString& address, const FUniqueNetIdRepl& uniqueId, FString& errorMessage)
{
    V_TRACE_MARKER();
    Super::PreLogin(options, address, uniqueId, errorMessage);
}

void ASteamFpsGameMode::PostLogin(APlayerController* player)
{
    V_TRACE_MARKER();
    Super::PostLogin(player);
}

void ASteamFpsGameMode::RestartPlayer(class AController* player)
{
    V_TRACE_MARKER();
    Super::RestartPlayer(player);
}

void ASteamFpsGameMode::Logout(AController* exiting)
{
    V_TRACE_MARKER();
    Super::Logout(exiting);
}

void ASteamFpsGameMode::HandleMatchHasStarted()
{
    V_TRACE_MARKER();
    Super::HandleMatchHasStarted();
}

void ASteamFpsGameMode::HandleMatchHasEnded()
{
    V_TRACE_MARKER();
    Super::HandleMatchHasEnded();

    auto ai = USteamFpsGameInstance::GetActorInstance();
    ai->SetDesiredState(eGameState::EndOfMatch);

    //auto gi = USteamFpsGameInstance::GetInstance();
    //V_CHECK_VALID(gi);
    //gi->OpenLevel("LobbyMap", true);
}

void ASteamFpsGameMode::HandleLeavingMap()
{
    V_TRACE_MARKER();
    Super::HandleLeavingMap();
}

