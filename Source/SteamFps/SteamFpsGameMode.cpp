// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsGameInstance.h"
#include "SteamFpsGameMode.h"
#include "SteamFpsGameSession.h"
#include "SteamFpsGameState.h"
#include "Gameplay/SteamFpsPlayerController.h"
#include "Gameplay/SteamFpsHUD.h"
#include "Gameplay/SteamFpsCharacter.h"
#include "Gameplay/SteamFpsPlayerState.h"

ASteamFpsGameMode::ASteamFpsGameMode()
{
    V_TRACE_MARKER();

    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/FirstPersonCharacter"));
    DefaultPawnClass = PlayerPawnClassFinder.Class;

    // use our custom HUD class
    HUDClass = ASteamFpsHUD::StaticClass();

    GameStateClass = ASteamFpsGameState::StaticClass();
    PlayerControllerClass = ASteamFpsPlayerController::StaticClass();
    PlayerStateClass = ASteamFpsPlayerState::StaticClass();
}

void ASteamFpsGameMode::PreInitializeComponents()
{
    V_TRACE_MARKER();
    Super::PreInitializeComponents();

    GetWorldTimerManager().SetTimer(m_defaultTimerHandle, this, &ASteamFpsGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ASteamFpsGameMode::BeginPlay()
{
    V_TRACE_MARKER();
    Super::BeginPlay();

    //ExecuteOnceAfter("CallEndMatch", 15.0f, [=]() { this->EndMatch(); });

    ExecuteOnPeriod("ReportGameState", 5.0f, [=]() {
        auto ms = GetMatchState();
        V_LOG_S(GeneralLog, "GameState=%s", *(ms.ToString()));
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
    //Super::PreLogin(options, address, uniqueId, errorMessage);

    ASteamFpsGameState* const MyGameState = Cast<ASteamFpsGameState>(GameState);
    const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
    if (bMatchIsOver)
    {
        errorMessage = TEXT("Match is over!");
    }
    else
    {
        // GameSession can be NULL if the match is over
        Super::PreLogin(options, address, uniqueId, errorMessage);
    }
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


void ASteamFpsGameMode::HandleMatchIsWaitingToStart()
{
    V_TRACE_MARKER();
    Super::HandleMatchIsWaitingToStart();

    //if (bNeedsBotCreation)
    //{
    //    CreateBotControllers();
    //    bNeedsBotCreation = false;
    //}

    if (bDelayedStart)
    {
        // start warmup if needed
        auto gs = Cast<ASteamFpsGameState>(GameState);
        if (IsValid(gs) && gs->m_remainingTime == 0)
        {
            const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
            if (bWantsMatchWarmup && WarmupTime > 0)
            {
                gs->m_remainingTime = WarmupTime;
            }
            else
            {
                gs->m_remainingTime = 0.0f;
            }
        }
    }
}

void ASteamFpsGameMode::HandleMatchHasStarted()
{
    V_TRACE_MARKER();
    Super::HandleMatchHasStarted();

    //bNeedsBotCreation = true;

    auto gs = Cast<ASteamFpsGameState>(GameState);
    //gs->RemainingTime = RoundTime;
    //StartBots();

    // notify players
    for (auto&& it = GetWorld()->GetControllerIterator(); it; ++it)
    {
        auto pc = Cast<ASteamFpsPlayerController>(*it);
        if (IsValid(pc))
        {
            pc->ClientGameStarted();
        }
    }
}

void ASteamFpsGameMode::HandleMatchHasEnded()
{
    V_TRACE_MARKER();
    Super::HandleMatchHasEnded();

    auto ai = USteamFpsGameInstance::GetActorInstance();
    ai->SetDesiredState(eGameState::EndOfMatch);
}

void ASteamFpsGameMode::HandleLeavingMap()
{
    V_TRACE_MARKER();
    Super::HandleLeavingMap();
}

void ASteamFpsGameMode::DefaultTimer()
{
    // don't update timers for Play In Editor mode, it's not real match
    if (GetWorld()->IsPlayInEditor())
    {
        // start match if necessary.
        if (GetMatchState() == MatchState::WaitingToStart)
        {
            StartMatch();
        }
        return;
    }

    auto gs = Cast<ASteamFpsGameState>(GameState);
    if (IsValid(gs) && gs->m_remainingTime > 0 && !gs->m_timerPaused)
    {
        gs->m_remainingTime--;

        if (gs->m_remainingTime <= 0)
        {
            if (GetMatchState() == MatchState::WaitingPostMatch)
            {
                RestartGame();
            }
            else if (GetMatchState() == MatchState::InProgress)
            {
                FinishMatch();

                // Send end round events
                for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
                {
                    auto pc = Cast<ASteamFpsPlayerController>(*It);

                    if (IsValid(pc) && IsValid(gs)) // BUGBUG: gs check redundant??
                    {
                        auto ps = Cast<ASteamFpsPlayerState>((*It)->PlayerState);
                        const bool bIsWinner = IsWinner(ps);

                        pc->ClientSendRoundEndEvent(bIsWinner, gs->ElapsedTime);
                    }
                }
            }
            else if (GetMatchState() == MatchState::WaitingToStart)
            {
                StartMatch();
            }
        }
    }
}

void ASteamFpsGameMode::FinishMatch()
{
    auto gs = Cast<ASteamFpsGameState>(GameState);
    if (IsMatchInProgress())
    {
        EndMatch();
        DetermineMatchWinner();

        auto world = GetWorld();

        // notify players
        for (auto it = world->GetControllerIterator(); it; ++it)
        {
            auto ps = Cast<ASteamFpsPlayerState>((*it)->PlayerState);
            const bool bIsWinner = IsWinner(ps);

            (*it)->GameHasEnded(NULL, bIsWinner);
        }

        // lock all pawns
        // pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
        // turning these back on.
        for (auto it = world->GetPawnIterator(); it; ++it)
        {
            (*it)->TurnOff();
        }

        // set up to restart the match
        gs->m_remainingTime = TimeBetweenMatches;
    }
}

void ASteamFpsGameMode::DetermineMatchWinner()
{
    // nothing to do here
}

bool ASteamFpsGameMode::IsWinner(class ASteamFpsPlayerState* PlayerState) const
{
    return false;
}

void ASteamFpsGameMode::RequestFinishAndExitToMainMenu()
{
    FinishMatch();

    //XXX
    //auto gi = Cast<USteamFpsGameInstance>(GetGameInstance());
    //if (IsValid(gi))
    //{
    //    gi->RemoveSplitScreenPlayers();
    //}

    ASteamFpsPlayerController* localPrimaryController = nullptr;
    for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
    {
        auto ctrl = Cast<ASteamFpsPlayerController>(*it);
        if (!IsValid(ctrl))
        {
            continue;
        }

        if (!ctrl->IsLocalController())
        {
            const FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
            ctrl->ClientReturnToMainMenu(RemoteReturnReason);
        }
        else
        {
            localPrimaryController = ctrl;
        }
    }

    // GameInstance should be calling this from an EndState.  So call the PC function that performs cleanup, not the one that sets GI state.
    if (IsValid(localPrimaryController))
    {
        localPrimaryController->HandleReturnToMainMenu();
    }
}
