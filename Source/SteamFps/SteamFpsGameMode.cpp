// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsGameMode.h"
#include "Gameplay/SteamFpsHUD.h"
#include "SteamFpsGameSession.h"
#include "Gameplay/SteamFpsCharacter.h"

ASteamFpsGameMode::ASteamFpsGameMode()
{
    V_TRACE_MARKER();
}

void ASteamFpsGameMode::BeginPlay()
{
    Super::BeginPlay();
    V_TRACE_MARKER();

    m_modeTimer = 0.0f;
}

void ASteamFpsGameMode::Tick(float dT)
{
    Super::Tick(dT);

    m_modeTimer += dT;

    for(auto&& f: m_timers)
    {
        if (m_modeTimer < f->runAt)
        {
            continue;
        }

        if (f->runonce && f->executed)
        {
            // technically, we could remove this guy from the list.
            continue;
        }

        V_LOG(GeneralLog, "Execute: %s @ %.1f", *(f->name.ToString()), m_modeTimer);
        f->executed = true;
        f->func();
        f->runAt = m_modeTimer + f->period;
    }
}

TSubclassOf<AGameSession> ASteamFpsGameMode::GetGameSessionClass() const
{
    V_TRACE_MARKER();
    return ASteamFpsGameSession::StaticClass();
}

void ASteamFpsGameMode::ExecuteOnceAfter(FName name, float time, std::function<void()> func)
{
    m_timers.emplace_back(std::make_shared<TimedCallback>(name, true, time, func));
}

void ASteamFpsGameMode::ExecuteOnPeriod(FName name, float time, std::function<void()> func)
{
    m_timers.emplace_back(std::make_shared<TimedCallback>(name, false, time, func));
}
