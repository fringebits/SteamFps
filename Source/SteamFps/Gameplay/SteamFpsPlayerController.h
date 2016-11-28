// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Online.h"
#include "UI/SteamFpsHUD.h"
#include "SteamFpsPlayerController.generated.h"

UCLASS()
class STEAMFPS_API ASteamFpsPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    /** notify player about started match */
    UFUNCTION(reliable, client)
        void ClientGameStarted();

    /** Starts the online game using the session name in the PlayerState */
    UFUNCTION(reliable, client)
        void ClientStartOnlineGame();

    /** Ends the online game using the session name in the PlayerState */
    UFUNCTION(reliable, client)
        void ClientEndOnlineGame();

    /** Notifies clients to send the end-of-round event */
    UFUNCTION(reliable, client)
        void ClientSendRoundEndEvent(bool bIsWinner, int32 ExpendedTimeInSeconds);

    /** notify local client about deaths */
    void OnDeathMessage(class ASteamFpsPlayerState* KillerPlayerState, class ASteamFpsPlayerState* KilledPlayerState, const UDamageType* KillerDamageType);

    /** Informs that player fragged someone */
    void OnKill();

    /** Cleans up any resources necessary to return to main menu.  Does not modify GameInstance state. */
    virtual void HandleReturnToMainMenu();

    /** Associate a new UPlayer with this PlayerController. */
    virtual void SetPlayer(UPlayer* Player);

    virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

protected:
    class ASteamFpsHUD* GetSteamFpsHud() { return Cast<ASteamFpsHUD>(GetHUD()); }

protected:
    /** if set, gameplay related actions (movement, weapn usage, etc) are allowed */
    uint8 bAllowGameActions : 1;

    /** true for the first frame after the game has ended */
    uint8 bGameEndedFrame : 1;

    FName	ServerSayString;

    // Timer used for updating friends in the player tick.
    float ShooterFriendUpdateTimer;

    // For tracking whether or not to send the end event
    bool bHasSentStartEvents;

    /** Handle for efficient management of ClientStartOnlineGame timer */
    FTimerHandle TimerHandle_ClientStartOnlineGame;
};
