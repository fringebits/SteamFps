// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include <GameFramework/GameMode.h>
#include "Common/TimedCallback.h"
#include "Common/StateMachine.h"
#include "SteamFpsGameSession.h"
#include "SteamFpsGameMode.generated.h"

UCLASS()
class ASteamFpsGameMode
    : public AGameMode
    , public CallbackComponent
{
	GENERATED_BODY()

public:
	ASteamFpsGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float dT) override;

    virtual TSubclassOf<AGameSession> GetGameSessionClass() const override 
    { 
        return ASteamFpsGameSession::StaticClass();
    }

protected:
    // Init game, initialize parameters and spawn helper classes
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    // Accepts or rejects player attempting to join server.
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

    // Called after successful login.  First place we can safely call replicated functions on the PlayerController.
    virtual void PostLogin(APlayerController* NewPlayer);

    // Called after PostLogin.  Default creates a pawn for the player.
    //virtual void HandleStartingNewPlayer() override;

    // Restart player.  Called to start spawning player's Pawn.
    virtual void RestartPlayer(class AController* NewPlayer) override;

    // This is what spawns the player's pawn
    //virtual void SpawnDefaultPawnAtTransform() override;

    // Called when player leaves or is destroyed.
    virtual void Logout(AController* Exiting) override;

    virtual void HandleMatchHasStarted() override;
    virtual void HandleMatchHasEnded() override;
    virtual void HandleLeavingMap() override;
};