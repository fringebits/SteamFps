// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsGameInstance.h"
#include "SteamFpsGameState.h"
#include "SteamFpsPlayerState.h"
#include "SteamFpsPlayerController.h"
#include "SteamFpsCharacter.h"

void ASteamFpsPlayerController::ClientGameStarted_Implementation()
{
    bAllowGameActions = true;

    // Enable controls mode now the game has started
    SetIgnoreMoveInput(false);

    //auto hud = GetSteamFpsHud();
    //if (IsValid(hud))
    //{
    //    hud->SetMatchState(EShooterMatchState::Playing);
    //    hud->ShowScoreboard(false);
    //}

    bGameEndedFrame = false;

    //QueryAchievements();

    // Send round start event
    const auto Events = Online::GetEventsInterface();
    auto LocalPlayer = Cast<ULocalPlayer>(Player);

    if (IsValid(LocalPlayer) && Events.IsValid())
    {
        auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

        if (UniqueId.IsValid())
        {
            // Generate a new session id
            Events->SetPlayerSessionId(*UniqueId, FGuid::NewGuid());

            FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());

            // Fire session start event for all cases
            FOnlineEventParms Params;
            Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
            Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
            Params.Add(TEXT("MapName"), FVariantData(MapName));

            Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionStart"), Params);

            // Online matches require the MultiplayerRoundStart event as well
            auto world = GetWorld();
            auto SGI = IsValid(world) ? Cast<USteamFpsGameInstance>(world->GetGameInstance()) : nullptr;
            //UShooterGameInstance* SGI = GetWorld() != NULL ? Cast<UShooterGameInstance>(GetWorld()->GetGameInstance()) : NULL;

            if (SGI->GetIsOnline())
            {
                FOnlineEventParms MultiplayerParams;

                // @todo: fill in with real values
                MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
                MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
                MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
                MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

                Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundStart"), MultiplayerParams);
            }

            bHasSentStartEvents = true;
        }
    }
}

/** Starts the online game using the session name in the PlayerState */
void ASteamFpsPlayerController::ClientStartOnlineGame_Implementation()
{
    V_CHECK(IsPrimaryPlayer());

    auto ps = Cast<ASteamFpsPlayerState>(PlayerState);
    if (IsValid(ps))
    {
        auto oss = IOnlineSubsystem::Get();
        if (oss != nullptr)
        {
            auto sessions = oss->GetSessionInterface();
            if (sessions.IsValid())
            {
                V_LOG(LogOnline, "Starting session %s on client", *ps->SessionName.ToString());
                sessions->StartSession(ps->SessionName);
            }
        }
    }
    else
    {
        // Keep retrying until player state is replicated
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_ClientStartOnlineGame, this, &ASteamFpsPlayerController::ClientStartOnlineGame_Implementation, 0.2f, false);
    }
}

/** Ends the online game using the session name in the PlayerState */
void ASteamFpsPlayerController::ClientEndOnlineGame_Implementation()
{
    V_TRACE_MARKER();
    V_CHECK(IsPrimaryPlayer());

    auto ps = Cast<ASteamFpsPlayerState>(PlayerState);
    if (IsValid(ps))
    {
        auto oss = IOnlineSubsystem::Get();
        if (oss != nullptr)
        {
            auto sessions = oss->GetSessionInterface();
            if (sessions.IsValid())
            {
                V_LOG(LogOnline, "Ending session %s on client", *ps->SessionName.ToString());
                sessions->EndSession(ps->SessionName);
            }
        }
    }
}

void ASteamFpsPlayerController::ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds)
{
    const auto Events = Online::GetEventsInterface();
    auto LocalPlayer = Cast<ULocalPlayer>(Player);

    if (bHasSentStartEvents && LocalPlayer != nullptr && Events.IsValid())
    {
        auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

        if (UniqueId.IsValid())
        {
            auto mapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());
            auto ps = Cast<ASteamFpsPlayerState>(PlayerState);
            auto score = IsValid(ps) ? ps->GetScore() : 0;

            // Fire session end event for all cases
            FOnlineEventParms Params;
            Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
            Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
            Params.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused
            Params.Add(TEXT("PlayerScore"), FVariantData((int32)score));
            Params.Add(TEXT("PlayerWon"), FVariantData((bool)bIsWinner));
            Params.Add(TEXT("MapName"), FVariantData(mapName));
            Params.Add(TEXT("MapNameString"), FVariantData(mapName)); // @todo workaround for a bug in backend service, remove when fixed

            Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionEnd"), Params);

            // Online matches require the MultiplayerRoundEnd event as well
            auto SGI = GetWorld() != NULL ? Cast<USteamFpsGameInstance>(GetWorld()->GetGameInstance()) : NULL;
            if (SGI->GetIsOnline())
            {
                FOnlineEventParms MultiplayerParams;

                auto MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<ASteamFpsGameState>() : NULL;
                if (ensure(MyGameState != nullptr))
                {
                    MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
                    MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
                    MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
                    MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
                    MultiplayerParams.Add(TEXT("TimeInSeconds"), FVariantData((float)ExpendedTimeInSeconds));
                    MultiplayerParams.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused

                    Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundEnd"), MultiplayerParams);
                }
            }
        }

        bHasSentStartEvents = false;
    }
}

void ASteamFpsPlayerController::OnKill()
{
    //UpdateAchievementProgress(ACH_FRAG_SOMEONE, 100.0f);

    const auto Events = Online::GetEventsInterface();
    const auto Identity = Online::GetIdentityInterface();

    if (Events.IsValid() && Identity.IsValid())
    {
        ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
        if (LocalPlayer)
        {
            int32 UserIndex = LocalPlayer->GetControllerId();
            TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
            if (UniqueID.IsValid())
            {
                auto ShooterChar = Cast<ASteamFpsCharacter>(GetCharacter());

                // If player is dead, use location stored during pawn cleanup.
                FVector Location = ShooterChar ? ShooterChar->GetActorLocation() : FVector::ZeroVector;
                //ASteamFpsWeapon* Weapon = ShooterChar ? ShooterChar->GetWeapon() : 0;
                //int32 WeaponType = Weapon ? (int32)Weapon->GetAmmoType() : 0;
                int32 weaponType = 0;

                FOnlineEventParms Params;

                Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
                Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
                Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

                Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0)); // unused
                Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)weaponType));
                Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0)); // unused
                Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0)); // untracked			
                Params.Add(TEXT("KillTypeId"), FVariantData((int32)0)); // unused
                Params.Add(TEXT("LocationX"), FVariantData(Location.X));
                Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
                Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));

                Events->TriggerEvent(*UniqueID, TEXT("KillOponent"), Params);
            }
        }
    }
}

void ASteamFpsPlayerController::OnDeathMessage(class ASteamFpsPlayerState* KillerPlayerState, class ASteamFpsPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{
    //ASteamFpsHUD* ShooterHUD = GetShooterHUD();
    //if (ShooterHUD)
    //{
    //    ShooterHUD->ShowDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
    //}

    ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
    if (LocalPlayer && LocalPlayer->GetCachedUniqueNetId().IsValid() && KilledPlayerState->UniqueId.IsValid())
    {
        // if this controller is the player who died, update the hero stat.
        if (*LocalPlayer->GetCachedUniqueNetId() == *KilledPlayerState->UniqueId)
        {
            const auto Events = Online::GetEventsInterface();
            const auto Identity = Online::GetIdentityInterface();

            if (Events.IsValid() && Identity.IsValid())
            {
                const int32 UserIndex = LocalPlayer->GetControllerId();
                TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
                if (UniqueID.IsValid())
                {
                    ASteamFpsCharacter* ShooterChar = Cast<ASteamFpsCharacter>(GetCharacter());

                    FVector Location = ShooterChar ? ShooterChar->GetActorLocation() : FVector::ZeroVector;
                    //ASteamFpsWeapon* Weapon = ShooterChar ? ShooterChar->GetWeapon() : NULL;
                    int32 weaponType = 0; //Weapon ? (int32)Weapon->GetAmmoType() : 0;

                    FOnlineEventParms Params;
                    Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
                    Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
                    Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

                    Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0)); // unused
                    Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)weaponType));
                    Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0)); // unused
                    Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0)); // untracked

                    Params.Add(TEXT("LocationX"), FVariantData(Location.X));
                    Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
                    Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));

                    Events->TriggerEvent(*UniqueID, TEXT("PlayerDeath"), Params);
                }
            }
        }
    }
}

void ASteamFpsPlayerController::HandleReturnToMainMenu()
{
    V_TRACE_MARKER();

    //OnHideScoreboard();
    //CleanupSessionOnReturnToMenu();
}

void ASteamFpsPlayerController::SetPlayer(UPlayer* InPlayer)
{
    V_TRACE_MARKER();
    Super::SetPlayer(InPlayer);

    //Build menu only after game is initialized
    //TODO: Need to create menu.
    //ShooterIngameMenu = MakeShareable(new FShooterIngameMenu());
    //ShooterIngameMenu->Construct(Cast<ULocalPlayer>(Player));

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
}

void ASteamFpsPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
    V_TRACE_MARKER();
    Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);

    auto world = GetWorld();
    V_CHECK_VALID(world);

    //auto viewport = Cast<USteamFpsGameViewportClient>(GetWorld()->GetGameViewport());
    //if (IsValid(viewport))
    //{
    //    ShooterViewport->ShowLoadingScreen();
    //}

    //auto hud = Cast<ASteamFpsHUD>(GetHUD());
    //if (IsValid(hud))
    //{
    //    // Passing true to bFocus here ensures that focus is returned to the game viewport.
    //    ShooterHUD->ShowScoreboard(false, true);
    //}
}
