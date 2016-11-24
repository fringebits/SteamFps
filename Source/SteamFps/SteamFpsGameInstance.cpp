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
    : m_gameActor(nullptr)
{
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

ASteamFpsGameActor* USteamFpsGameInstance::GetActorInstance()
{
    auto gi = GetInstance();
    return (gi != nullptr) ? gi->m_gameActor : nullptr;
}

void USteamFpsGameInstance::Init()
{
    V_TRACE_MARKER();

    auto world = GetWorld();
    V_CHECK_VALID(world);

    m_gameActor = world->SpawnActor<ASteamFpsGameActor>();
}

void USteamFpsGameInstance::Shutdown()
{
    V_TRACE_MARKER();

    V_CHECK_VALID(m_gameActor);
    m_gameActor->Destroy();
}

