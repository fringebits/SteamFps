#pragma once

#include "SteamFps.h"
#include "Logging.h"

DEFINE_LOG_CATEGORY(General)
DEFINE_LOG_CATEGORY(AssertLog)
DEFINE_LOG_CATEGORY(LogTrace)
DEFINE_LOG_CATEGORY(LogTest)
DEFINE_LOG_CATEGORY(GameSessionLog)
DEFINE_LOG_CATEGORY(GameModeLog)
DEFINE_LOG_CATEGORY(GameStateLog)
DEFINE_LOG_CATEGORY(WeaponLog)
DEFINE_LOG_CATEGORY(AbilityLog)

namespace LoggingHelpers
{
    static auto const format = TEXT("[this: 0x%08x] [Name=%s] [Role=%s]");

    FString FORMAT_LOG_SCOPE_ACTOR(void const* const actor)
    {
        auto THIS = static_cast<AActor const* const>(actor);
        return FString::Printf(format, static_cast<void const*>(THIS), *GetNameSafe(THIS), IsValid(THIS) ? V_FORMAT_ROLE(THIS->Role) : TEXT("Invalid"));
    }

    FString FORMAT_LOG_SCOPE_COMPONENT(void const* const component)
    {
        auto THIS = static_cast<UActorComponent const* const>(component);
        return FString::Printf(format, static_cast<void const*>(THIS), *GetNameSafe(THIS), (IsValid(THIS) && IsValid(THIS->GetOwner())) ? V_FORMAT_ROLE(THIS->GetOwner()->Role) : TEXT("This Component has no owner"));
    }

    void LogToScreen(const char* message)
    {
        auto fmsg = FString::Printf(TEXT("%s"), message);
        GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, fmsg);
    }

    Trace::Trace(const FString& loc, bool logExit, const FString& scope)
        : m_loc(loc)
        , m_logExit(logExit)
    {
        if (m_logExit)
        {
            UE_LOG(LogTrace, Log, TEXT("==> %s %s"), *m_loc, *scope);
            m_start = std::chrono::high_resolution_clock::now();
        }
        else
        {
            UE_LOG(LogTrace, Log, TEXT("%s %s"), *m_loc, *scope);
        }
    }

    Trace::~Trace()
    {
        if (!m_logExit)
        {
            return;
        }

        auto span = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start);
        auto count = span.count();
        UE_LOG(LogTrace, Log, TEXT("<== %s %dms"), *m_loc, count);
    }
}
