// Fill out your copyright notice in the Description page of Project Settings.

#include "SteamFps.h"
#include <Blueprint/UserWidget.h>
#include <OnlineSessionInterface.h>
#include <Online.h>
#include "SteamFpsGameInstance.h"

USteamFpsGameInstance::USteamFpsGameInstance()
    : m_enableLan(false)
{
    m_stateData.AddZeroed(static_cast<int>(eGameState::kCount));
}

void USteamFpsGameInstance::CreateWidgets()
{
    // Error dialog
}

void USteamFpsGameInstance::ShowWidget(eGameState gs, bool flag)
{
    auto widget = m_stateData[static_cast<int>(gs)].Widget;

    if (widget == nullptr)
    {
        return;
    }

    if (flag)
    {
        // TODO: AddToViewport(widget);
    }
    else
    {
        // Hide the widget
        widget->RemoveFromParent();
    }
}

void USteamFpsGameInstance::StateTransition(eGameState gs)
{
    if (IsInState(gs))
    {
        // PrintToScreen("Error, trying to leaving curState.", Red, 5.0s);
        return;
    }

    // Handle exiting existing states
    ShowWidget(m_currentState, false);

    switch(m_currentState)
    {
    case eGameState::Playing:
        // TODO: DestroySession(GetPlayerController(0)) // VIDEO@(28min)
        break;
    }

    // PrintToScreen("Setting new state {gs}", White, 5.0f);
    m_currentState = gs;
    ShowWidget(m_currentState, true);
}

bool USteamFpsGameInstance::ServerTravel(const FString& url, bool absolute, bool shouldSkipGameNotify)
{
    V_TRACE();
    V_LOG(General, "ServerTravel(%s, %s, %s)", *url, V_FORMAT_BOOL(absolute), V_FORMAT_BOOL(shouldSkipGameNotify));
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerTravel(%s)"), *url), true, false, FColor::White, 5);

    auto world = GetWorld();
    //V_ASSERT(IsValid(world), "Invalid world.");
    auto ret = world->ServerTravel(url, absolute, shouldSkipGameNotify);
    return ret;
}

void USteamFpsGameInstance::ShowLoadScreen()
{
}

void USteamFpsGameInstance::HostGameEvent()
{
    ShowLoadScreen();
    CreateSession();
    // TODO: OpenLevel("MP_Level1", Absolute, Listen);
}

void USteamFpsGameInstance::DestroySession(APlayerController* playerController)
{
}

void USteamFpsGameInstance::CreateSession()
{
    auto sessionInterface = Online::GetSessionInterface();
    V_CHECK(sessionInterface.IsValid());

    auto sessionState = sessionInterface->GetSessionState(SESSION_NAME);
    V_LOG(General, "SessionState = %s", *V_FORMAT_ENUM(EOnlineSessionState::Type, sessionState));

    if (sessionState == EOnlineSessionState::NoSession)
    {
        FOnlineSessionSettings settings;
        settings.NumPublicConnections = M_MAX_PLAYER_COUNT;
        settings.bShouldAdvertise = true;
        settings.bAllowJoinInProgress = false;
        settings.bIsLANMatch = m_enableLan;
        settings.bUsesPresence = true;
        settings.bAllowJoinViaPresence = true;
        settings.bIsDedicated = true;

        auto sessionResult = sessionInterface->CreateSession(0, SESSION_NAME, settings);
        if (!sessionResult)
        {
            V_ERROR(General, "CreateSession failed.");
        }
    }
}

void USteamFpsGameInstance::ErrorHandler()
{
    // StateTransition: ErrorState
    // DestroySession();
}

void USteamFpsGameInstance::ShowMainMenu()
{
}
