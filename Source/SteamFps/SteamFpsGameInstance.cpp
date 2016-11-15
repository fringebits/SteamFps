// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include <Blueprint/UserWidget.h>
#include <OnlineSessionInterface.h>
#include <Online.h>
#include "SteamFpsGameInstance.h"

namespace
{
    std::list<USteamFpsGameInstance*> s_instanceList;
}

USteamFpsGameInstance::USteamFpsGameInstance()
    : m_enableLan(false)
{
    m_stateData.AddZeroed(static_cast<int>(eGameState::kCount));
    s_instanceList.emplace_back(this);
}

USteamFpsGameInstance::~USteamFpsGameInstance()
{
    s_instanceList.remove(this);
}

USteamFpsGameInstance* USteamFpsGameInstance::GetInstance()
{
    return !s_instanceList.empty() ? s_instanceList.back() : nullptr;
}

void USteamFpsGameInstance::OpenLevel(FName levelName, bool absolute, FString options)
{
    V_LOG(GeneralLog, "OpenLevel(%s, %s, %s)", *levelName.ToString(), V_FORMAT_BOOL(absolute), *options);
    auto world = this->GetWorld();
    UGameplayStatics::OpenLevel(this, levelName, absolute, options);
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

    OnExitGameState(m_currentState);
    m_currentState = gs;
    OnEnterGameState(m_currentState);
}

void USteamFpsGameInstance::OnEnterGameState(const eGameState gs)
{
    // PrintToScreen("Setting new state {gs}", White, 5.0f);
    ShowWidget(gs, true);
}

void USteamFpsGameInstance::OnExitGameState(const eGameState gs)
{
    ShowWidget(gs, false);

    switch (gs)
    {
    case eGameState::Playing:
        // TODO: DestroySession(GetPlayerController(0)) // VIDEO@(28min)
        break;
    }
}

bool USteamFpsGameInstance::ServerTravel(const FString& url, bool absolute, bool shouldSkipGameNotify)
{
    V_TRACE();
    V_LOG(GeneralLog, "ServerTravel(%s, %s, %s)", *url, V_FORMAT_BOOL(absolute), V_FORMAT_BOOL(shouldSkipGameNotify));
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
    //CreateSession();
    // TODO: OpenLevel("MP_Level1", Absolute, Listen);
}

void USteamFpsGameInstance::DestroySession(APlayerController* playerController)
{
}

void USteamFpsGameInstance::ErrorHandler()
{
    // StateTransition: ErrorState
    // DestroySession();
}

void USteamFpsGameInstance::ShowMainMenu()
{
}