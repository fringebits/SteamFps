// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include <Blueprint/UserWidget.h>
#include <OnlineSessionInterface.h>
#include <Online.h>
#include "SteamFpsGameInstance.h"
#include "SteamFpsGameSession.h"
#include "SteamFpsGameActor.h"

ASteamFpsGameActor::ASteamFpsGameActor()
    : m_enableLan(true)
{
    m_stateData.AddZeroed(static_cast<int>(eGameState::kCount));
}

void ASteamFpsGameActor::Tick(float dT)
{
    Super::Tick(dT);
    TickMachine(dT);
}

void ASteamFpsGameActor::OpenLevel(FName levelName, bool absolute, FString options)
{
    V_LOG(GeneralLog, "OpenLevel(%s, %s, %s)", *levelName.ToString(), V_FORMAT_BOOL(absolute), *options);
    auto world = this->GetWorld();
    UGameplayStatics::OpenLevel(this, levelName, absolute, options);
}

void ASteamFpsGameActor::ShowWidget(eGameState gs, bool flag)
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

bool ASteamFpsGameActor::ServerTravel(const FString& url, bool absolute, bool shouldSkipGameNotify)
{
    V_TRACE();
    V_LOG(GeneralLog, "ServerTravel(%s, %s, %s)", *url, V_FORMAT_BOOL(absolute), V_FORMAT_BOOL(shouldSkipGameNotify));
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerTravel(%s)"), *url), true, false, FColor::White, 5);

    auto world = GetWorld();
    //V_ASSERT(IsValid(world), "Invalid world.");
    auto ret = world->ServerTravel(url, absolute, shouldSkipGameNotify);
    return ret;
}

void ASteamFpsGameActor::DestroySession(APlayerController* playerController)
{
}

void ASteamFpsGameActor::TickState(const eGameState gs, float dT)
{
}

void ASteamFpsGameActor::OnEnterState(const eGameState gs)
{
    ShowWidget(gs, true);

    switch (gs)
    {
    case eGameState::ErrorDialog:
        // DestroySession();
        break;
    }
}

void ASteamFpsGameActor::OnExitState(const eGameState gs)
{
    ShowWidget(gs, false);

    switch (gs)
    {
    case eGameState::Playing:
        // TODO: DestroySession(GetPlayerController(0)) // VIDEO@(28min)
        break;
    }
}