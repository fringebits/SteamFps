// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include <OnlineSubsystemUtils.h>
#include "SteamFpsGameInstance.h"
#include "SteamFpsGameSession.h"

ASteamFpsGameSession::ASteamFpsGameSession()
    : m_isLanMatch(false)
{
    V_TRACE_MARKER();
}

void ASteamFpsGameSession::BeginPlay()
{
    Super::BeginPlay();
    V_TRACE_MARKER();
}

void ASteamFpsGameSession::CreateOnlineSession()
{
    Super::RegisterServer();

    V_TRACE();

    auto sessionInterface = Online::GetSessionInterface();

    V_CHECK(sessionInterface.IsValid());

    auto sessionState = sessionInterface->GetSessionState(V_SESSION_NAME);
    V_LOG(GeneralLog, "SessionState = %s", EOnlineSessionState::ToString(sessionState));

    if (sessionState == EOnlineSessionState::NoSession)
    {
        auto delegateObject = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ASteamFpsGameSession::OnCreateSessionComplete);
        auto delegateHandler = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(delegateObject);

        FOnlineSessionSettings settings;

        settings.NumPublicConnections = V_MAX_PLAYER_COUNT;
        settings.bShouldAdvertise = true;
        settings.bAllowJoinInProgress = true;
        settings.bIsLANMatch = m_isLanMatch;
        settings.bUsesPresence = true;
        settings.bAllowJoinViaPresence = true;
        settings.bIsDedicated = true;

        auto sessionResult = sessionInterface->CreateSession(0, V_SESSION_NAME, settings);
        V_LOG_IF(sessionResult, GeneralLog, "Unable to create online session on server!");
    }
}

void ASteamFpsGameSession::OnCreateSessionComplete(FName sessionName, bool wasSuccessful)
{
    V_LOG(GeneralLog, "OnCreateSessionComplete: %s", V_FORMAT_BOOL(wasSuccessful));

    // Session was created.
    auto ai = USteamFpsGameInstance::GetActorInstance();
    V_CHECK_VALID(ai);

    ai->OpenLevel("SandboxMap", true, "listen");
    //UGameplayStatics::OpenLevel(GetWorld(), "Sandbox", true, "listen");
}

