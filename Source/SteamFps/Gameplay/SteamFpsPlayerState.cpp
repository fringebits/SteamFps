// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsPlayerController.h"
#include "SteamFpsPlayerState.h"
#include "SteamFpsGameState.h"
#include "Gameplay/SteamFpsCharacter.h"

ASteamFpsPlayerState::ASteamFpsPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    TeamNumber = 0;
    NumKills = 0;
    NumDeaths = 0;
    NumBulletsFired = 0;
    NumRocketsFired = 0;
    bQuitter = false;
}

void ASteamFpsPlayerState::Reset()
{
    Super::Reset();

    //PlayerStates persist across seamless travel.  Keep the same teams as previous match.
    //SetTeamNum(0);
    NumKills = 0;
    NumDeaths = 0;
    NumBulletsFired = 0;
    NumRocketsFired = 0;
    bQuitter = false;
}

void ASteamFpsPlayerState::UnregisterPlayerWithSession()
{
    if (!bFromPreviousLevel)
    {
        Super::UnregisterPlayerWithSession();
    }
}

void ASteamFpsPlayerState::ClientInitialize(AController* controller)
{
    Super::ClientInitialize(controller);

    V_TRACE_MARKER();
    Super::ClientInitialize(controller);

    auto ctrl = Cast<ASteamFpsPlayerController>(controller);
    V_CHECK_VALID(ctrl);

    UpdateTeamColors();
}

void ASteamFpsPlayerState::SetTeamNum(int32 NewTeamNumber)
{
    TeamNumber = NewTeamNumber;

    UpdateTeamColors();
}

void ASteamFpsPlayerState::OnRep_TeamColor()
{
    UpdateTeamColors();
}

void ASteamFpsPlayerState::AddBulletsFired(int32 NumBullets)
{
    NumBulletsFired += NumBullets;
}

void ASteamFpsPlayerState::AddRocketsFired(int32 NumRockets)
{
    NumRocketsFired += NumRockets;
}

void ASteamFpsPlayerState::SetQuitter(bool bInQuitter)
{
    bQuitter = bInQuitter;
}

void ASteamFpsPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);

    ASteamFpsPlayerState* ShooterPlayer = Cast<ASteamFpsPlayerState>(PlayerState);
    if (ShooterPlayer)
    {
        ShooterPlayer->TeamNumber = TeamNumber;
    }
}

void ASteamFpsPlayerState::UpdateTeamColors()
{
    AController* OwnerController = Cast<AController>(GetOwner());
    if (OwnerController != NULL)
    {
        ASteamFpsCharacter* ShooterCharacter = Cast<ASteamFpsCharacter>(OwnerController->GetCharacter());
        if (ShooterCharacter != NULL)
        {
            // BUGUBG: Not implemented.
            //ShooterCharacter->UpdateTeamColorsAllMIDs();
        }
    }
}

int32 ASteamFpsPlayerState::GetTeamNum() const
{
    return TeamNumber;
}

int32 ASteamFpsPlayerState::GetKills() const
{
    return NumKills;
}

int32 ASteamFpsPlayerState::GetDeaths() const
{
    return NumDeaths;
}

float ASteamFpsPlayerState::GetScore() const
{
    return Score;
}

int32 ASteamFpsPlayerState::GetNumBulletsFired() const
{
    return NumBulletsFired;
}

int32 ASteamFpsPlayerState::GetNumRocketsFired() const
{
    return NumRocketsFired;
}

bool ASteamFpsPlayerState::IsQuitter() const
{
    return bQuitter;
}

void ASteamFpsPlayerState::ScoreKill(ASteamFpsPlayerState* Victim, int32 Points)
{
    NumKills++;
    ScorePoints(Points);
}

void ASteamFpsPlayerState::ScoreDeath(ASteamFpsPlayerState* KilledBy, int32 Points)
{
    NumDeaths++;
    ScorePoints(Points);
}

void ASteamFpsPlayerState::ScorePoints(int32 Points)
{
    ASteamFpsGameState* const MyGameState = Cast<ASteamFpsGameState>(GetWorld()->GameState);
    if (MyGameState && TeamNumber >= 0)
    {
        if (TeamNumber >= MyGameState->m_teamScores.Num())
        {
            MyGameState->m_teamScores.AddZeroed(TeamNumber - MyGameState->m_teamScores.Num() + 1);
        }

        MyGameState->m_teamScores[TeamNumber] += Points;
    }

    Score += Points;
}

void ASteamFpsPlayerState::InformAboutKill_Implementation(class ASteamFpsPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ASteamFpsPlayerState* KilledPlayerState)
{
    //id can be null for bots
    if (KillerPlayerState->UniqueId.IsValid())
    {
        //search for the actual killer before calling OnKill()	
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            ASteamFpsPlayerController* TestPC = Cast<ASteamFpsPlayerController>(*It);
            if (TestPC && TestPC->IsLocalController())
            {
                // a local player might not have an ID if it was created with CreateDebugPlayer.
                ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
                TSharedPtr<const FUniqueNetId> LocalID = LocalPlayer->GetCachedUniqueNetId();
                if (LocalID.IsValid() && *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->UniqueId)
                {
                    TestPC->OnKill();
                }
            }
        }
    }
}

void ASteamFpsPlayerState::BroadcastDeath_Implementation(class ASteamFpsPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ASteamFpsPlayerState* KilledPlayerState)
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        // all local players get death messages so they can update their huds.
        ASteamFpsPlayerController* TestPC = Cast<ASteamFpsPlayerController>(*It);
        if (TestPC && TestPC->IsLocalController())
        {
            TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);
        }
    }
}

void ASteamFpsPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASteamFpsPlayerState, TeamNumber);
    DOREPLIFETIME(ASteamFpsPlayerState, NumKills);
    DOREPLIFETIME(ASteamFpsPlayerState, NumDeaths);
}

FString ASteamFpsPlayerState::GetShortPlayerName() const
{
    if (PlayerName.Len() > V_MAX_NAME_LENGTH)
    {
        return PlayerName.Left(V_MAX_NAME_LENGTH) + "...";
    }
    return PlayerName;
}

