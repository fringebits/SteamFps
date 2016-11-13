// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include "Runtime/Core/Public/Misc/Paths.h"
#include <chrono>

DECLARE_LOG_CATEGORY_EXTERN(General, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(AssertLog, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(LogTrace, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(LogTest, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(GameSessionLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(GameModeLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(GameStateLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(WeaponLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(AbilityLog, Log, All);

#define V_FORMAT_NETMODE(world) \
!(IsValid(GEngine) && IsValid(world)) ? TEXT("World or GEngine not valid") : \
GEngine->GetNetMode(world) == NM_Client ? TEXT("Client") : \
GEngine->GetNetMode(world) == NM_ListenServer ? TEXT("ListenServer") : \
GEngine->GetNetMode(world) == NM_DedicatedServer ? TEXT("DedicatedServer") : \
GEngine->GetNetMode(world) == NM_Standalone ? TEXT("Standalone") : \
TEXT("InvalidNetMode")

// COLOR_BLACK = TEXT("0000");
// COLOR_DARK_RED = TEXT("1000");
// COLOR_DARK_GREEN = TEXT("0100");
// COLOR_DARK_BLUE = TEXT("0010");
// COLOR_DARK_YELLOW = TEXT("1100");
// COLOR_DARK_CYAN = TEXT("0110");
// COLOR_DARK_PURPLE = TEXT("1010");
// COLOR_DARK_WHITE = TEXT("1110");
// COLOR_GRAY = COLOR_DARK_WHITE;
// COLOR_RED = TEXT("1001");
// COLOR_GREEN = TEXT("0101");
// COLOR_BLUE = TEXT("0011");
// COLOR_YELLOW = TEXT("1101");
// COLOR_CYAN = TEXT("0111");
// COLOR_PURPLE = TEXT("1011");
// COLOR_WHITE = TEXT("1111");
// COLOR_NONE = TEXT("");

namespace LoggingHelpers
{
    FString FORMAT_LOG_SCOPE_ACTOR(void const* const actor);
    FString FORMAT_LOG_SCOPE_COMPONENT(void const* const component);
    void LogToScreen(const char* message);

    class Trace
    {
    public:
        Trace(const FString& loc, bool logExit, const FString& scope);
        ~Trace();

    private:
        const FString m_loc;
        bool m_logExit;
        std::chrono::high_resolution_clock::time_point m_start;
    };
}

#if !UE_BUILD_SHIPPING
// default colors for logging
#define X_BRIEF_COLOR_TEXT COLOR_WHITE
#define X_LOG_COLOR_TEXT COLOR_WHITE
#define X_LOG_COLOR_BACKGROUND COLOR_DARK_GREEN
#define X_WARNING_COLOR_TEXT COLOR_WHITE
#define X_WARNING_COLOR_BACKGROUND COLOR_DARK_YELLOW
#define X_ERROR_COLOR_TEXT COLOR_WHITE
#define X_ERROR_COLOR_BACKGROUND COLOR_DARK_RED

// format message contents
#define X_FORMAT_LOG_MESSAGE(format, ...) FString::Printf(TEXT(format), ##__VA_ARGS__)
#define X_FORMAT_LOG_LOCATION FString::Printf(TEXT("%s, %s(%d)"), TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__)
#define X_FORMAT_LOG_SCOPE_ACTOR LoggingHelpers::FORMAT_LOG_SCOPE_ACTOR(static_cast<void const* const>(this))
#define X_FORMAT_LOG_SCOPE_COMPONENT LoggingHelpers::FORMAT_LOG_SCOPE_COMPONENT(static_cast<void const* const>(this))
#define X_FORMAT_LOG_SCOPE_DEFAULT FString::Printf(TEXT("this=0x%08x"), static_cast<void const*>(this))
#define X_FORMAT_LOG_SCOPE (IsValid(Cast<AActor>(this)) ? X_FORMAT_LOG_SCOPE_ACTOR : (IsValid(Cast<UActorComponent>(this)) ? X_FORMAT_LOG_SCOPE_COMPONENT : X_FORMAT_LOG_SCOPE_DEFAULT))
#define X_FORMAT_LOG_CONDITION(condition) FString::Printf(TEXT("Condition=%s"), TEXT(#condition))

// format contents into an output
#define X_FORMAT_LOG_OUTPUT_BRIEF(format, ...) FString::Printf(TEXT("%s"), *X_FORMAT_LOG_MESSAGE(format, ##__VA_ARGS__))
#define X_FORMAT_LOG_OUTPUT_BRIEF_CONDITIONAL(condition, format, ...) FString::Printf(TEXT("%s [%s] [%s]"), *X_FORMAT_LOG_MESSAGE(format, ##__VA_ARGS__), *X_FORMAT_LOG_CONDITION(condition), *X_FORMAT_LOG_LOCATION)
#define X_FORMAT_LOG_OUTPUT_NU(format, ...) FString::Printf(TEXT("%s [%s]"), *X_FORMAT_LOG_MESSAGE(format, ##__VA_ARGS__), *X_FORMAT_LOG_LOCATION)
#define X_FORMAT_LOG_OUTPUT_NU_CONDITIONAL(condition, format, ...) X_FORMAT_LOG_OUTPUT_BRIEF_CONDITIONAL(condition, format, ##__VA_ARGS__)
#define X_FORMAT_LOG_OUTPUT_VERBOSE(format, ...) FString::Printf(TEXT("%s [%s] [%s]"), *X_FORMAT_LOG_MESSAGE(format, ##__VA_ARGS__), *X_FORMAT_LOG_LOCATION, *X_FORMAT_LOG_SCOPE)
#define X_FORMAT_LOG_OUTPUT_VERBOSE_CONDITIONAL(condition, format, ...) FString::Printf(TEXT("%s [%s] [%s] [%s]"), *X_FORMAT_LOG_MESSAGE(format, ##__VA_ARGS__), *X_FORMAT_LOG_CONDITION(condition), *X_FORMAT_LOG_LOCATION, *X_FORMAT_LOG_SCOPE)
#endif

#if !UE_BUILD_SHIPPING
// avoid using
#define X_LOG_BRIEF(category, color, background, format, ...) \
{ \
    SET_WARN_COLOR_AND_BACKGROUND(color, background); \
    UE_LOG(category, Log, TEXT("%s"), *X_FORMAT_LOG_OUTPUT_BRIEF(format, ##__VA_ARGS__)); \
    CLEAR_WARN_COLOR(); \
}

// avoid using
#define X_LOG_BRIEF_CONDITIONAL(condition, category, color, background, format, ...) \
{ \
    SET_WARN_COLOR_AND_BACKGROUND(color, background); \
    UE_LOG(category, Log, TEXT("%s"), *X_FORMAT_LOG_OUTPUT_BRIEF_CONDITIONAL(condition, format, ##__VA_ARGS__)); \
    CLEAR_WARN_COLOR(); \
}

#define X_LOG_NU(category, logType, color, background, format, ...) \
{ \
    SET_WARN_COLOR_AND_BACKGROUND(color, background); \
    UE_LOG(category, logType, TEXT("%s"), *X_FORMAT_LOG_OUTPUT_NU(format, ##__VA_ARGS__)); \
    CLEAR_WARN_COLOR(); \
}

#define X_LOG_NU_IF(condition, category, logType, color, background, format, ...) \
{ \
    SET_WARN_COLOR_AND_BACKGROUND(color, background); \
    UE_LOG(category, logType, TEXT("%s"), *X_FORMAT_LOG_OUTPUT_BRIEF_CONDITIONAL(condition, format, ##__VA_ARGS__)); \
    CLEAR_WARN_COLOR(); \
}

// avoid using
#define X_LOG_VERBOSE(category, logType, color, background, format, ...) \
{ \
    SET_WARN_COLOR_AND_BACKGROUND(color, background); \
    UE_LOG(category, logType, TEXT("%s"), *X_FORMAT_LOG_OUTPUT_VERBOSE(format, ##__VA_ARGS__)); \
    CLEAR_WARN_COLOR(); \
}

// avoid using
#define X_LOG_VERBOSE_CONDITIONAL(condition, category, logType, color, background, format, ...) \
{ \
    SET_WARN_COLOR_AND_BACKGROUND(color, background); \
    UE_LOG(category, logType, TEXT("%s"), *X_FORMAT_LOG_OUTPUT_VERBOSE_CONDITIONAL(condition, format, ##__VA_ARGS__)); \
    CLEAR_WARN_COLOR(); \
}
#else
#define X_LOG_BRIEF(category, color, background, format, ...) do{}while(false);
#define X_LOG_BRIEF_CONDITIONAL(condition, category, color, background, format, ...) do{}while(false);
#define X_LOG_VERBOSE(category, logType, color, background, format, ...) do{}while(false);
#define X_LOG_VERBOSE_CONDITIONAL(condition, category, logType, color, background, format, ...) do{}while(false);
#endif

#if !UE_BUILD_SHIPPING
// logging macros for use

// briefs can be used anywhere
#define V_BRIEF(category, color, format, ...) X_LOG_BRIEF(category, X_BRIEF_COLOR_TEXT, color, format, ##__VA_ARGS__)
#define V_BRIEF_IF(condition, category, color, format, ...) {if(condition){X_LOG_BRIEF_CONDITIONAL(condition, category, X_BRIEF_COLOR_TEXT, color, format, ##__VA_ARGS__);}}

// log, warning, and error can only be used if there is a this pointer
#define V_LOG(category, format, ...) X_LOG_VERBOSE(category, Log, X_LOG_COLOR_TEXT, X_LOG_COLOR_BACKGROUND, format, ##__VA_ARGS__)
#define V_LOG_IF(condition, category, format, ...) {if(condition){X_LOG_VERBOSE_CONDITIONAL(condition, category, Log, X_LOG_COLOR_TEXT, X_LOG_COLOR_BACKGROUND, format, ##__VA_ARGS__);}}
#define V_WARNING(category, format, ...) X_LOG_VERBOSE(category, Warning, X_WARNING_COLOR_TEXT, X_WARNING_COLOR_BACKGROUND, format, ##__VA_ARGS__)
#define V_WARNING_IF(condition, category, format, ...) {if(condition){X_LOG_VERBOSE_CONDITIONAL(condition, category, Warning, X_WARNING_COLOR_TEXT, X_WARNING_COLOR_BACKGROUND, format, ##__VA_ARGS__);}}
#define V_ERROR(category, format, ...) X_LOG_VERBOSE(category, Error, X_ERROR_COLOR_TEXT, X_ERROR_COLOR_BACKGROUND, format, ##__VA_ARGS__)
#define V_ERROR_IF(condition, category, format, ...) {if(condition){X_LOG_VERBOSE_CONDITIONAL(condition, category, Error, X_ERROR_COLOR_TEXT, X_ERROR_COLOR_BACKGROUND, format, ##__VA_ARGS__);}}

// log_s, warning_s, and error_s can be used anywhere and exisit to handle logging in global or static functions
#define V_LOG_S(category, format, ...) X_LOG_NU(category, Log, X_LOG_COLOR_TEXT, X_LOG_COLOR_BACKGROUND, format, ##__VA_ARGS__)
#define V_LOG_IF_S(condition, category, format, ...) {if(condition){X_LOG_NU_IF(condition, category, Log, X_LOG_COLOR_TEXT, X_LOG_COLOR_BACKGROUND, format, ##__VA_ARGS__)}}
#define V_WARNING_S(category, format, ...) X_LOG_NU(category, Warning, X_WARNING_COLOR_TEXT, X_WARNING_COLOR_BACKGROUND, format, ##__VA_ARGS__)
#define V_WARNING_IF_S(condition, category, format, ...) {if(condition){X_LOG_NU_IF(condition, category, Warning, X_WARNING_COLOR_TEXT, X_WARNING_COLOR_BACKGROUND, format, ##__VA_ARGS__)}}
#define V_ERROR_S(category, format, ...) X_LOG_NU(category, Error, X_ERROR_COLOR_TEXT, X_ERROR_COLOR_BACKGROUND, format, ##__VA_ARGS__)
#define V_ERROR_IF_S(condition, category, format, ...) {if(condition){X_LOG_NU_IF(condition, category, Error, X_ERROR_COLOR_TEXT, X_ERROR_COLOR_BACKGROUND, format, ##__VA_ARGS__)}}

// simple trace utilities for logging scope in/out (optionally with timer)
#define V_TRACE()           LoggingHelpers::Trace _trace ## __LINE__(X_FORMAT_LOG_LOCATION, true, X_FORMAT_LOG_SCOPE)
#define V_TRACE_MARKER()    LoggingHelpers::Trace _trace ## __LINE__(X_FORMAT_LOG_LOCATION, false, X_FORMAT_LOG_SCOPE)
#define V_DEPRECATED()      V_WARNING_S(General, "This funcion is deprecated.")
#define V_NOT_IMPLEMENTED() V_ERROR_S(General, "This funcion has not been implemented.")

#else
#define V_BRIEF(category, color, format, ...) do{}while(false);
#define V_BRIEF_IF(condition, category, color, format, ...) do{}while(false);
#define V_LOG(category, format, ...) do{}while(false);
#define V_LOG_IF(condition, category, format, ...) do{}while(false);
#define V_WARNING(category, format, ...) do{}while(false);
#define V_WARNING_IF(condition, category, format, ...) do{}while(false);
#define V_ERROR(category, format, ...) do{}while(false);
#define V_ERROR_IF(condition, category, format, ...) do{}while(false);
#define V_LOG_S(category, format, ...) do{}while(false);
#define V_LOG_IF_S(condition, category, format, ...) do{}while(false);
#define V_WARNING_S(category, format, ...) do{}while(false);
#define V_WARNING_IF_S(condition, category, format, ...) do{}while(false);
#define V_ERROR_S(category, format, ...) do{}while(false);
#define V_ERROR_IF_S(condition, category, format, ...) do{}while(false);

#define V_TRACE()
#define V_TRACE_MARKER()
#define V_DEPRECATED()
#define V_NOT_IMPLEMENTED()

#endif

// Macros for checking / requiring conditions
#define V_CHECK(condition) \
    do { if (!(condition)) { \
        V_ERROR_IF_S(!(condition), General, "V_CHECK failed!"); \
        LoggingHelpers::LogToScreen(#condition); \
        return; \
    } } while (true, false)

#define V_CHECK_RETURN(condition, result) \
    do { if (!(condition)) { \
        V_ERROR_IF_S(!(condition), General, "V_CHECK failed!"); \
        LoggingHelpers::LogToScreen(#condition); \
        return (result); \
    } } while (true, false)

#define V_CHECK_NULL(pointer) \
    V_CHECK(pointer != nullptr);

#define V_CHECK_NULL_RETURN(pointer, result) \
    V_CHECK_RETURN(pointer != nullptr, result);

#define V_CHECK_VALID(object) \
    V_CHECK(IsValid(object));

#define V_CHECK_VALID_RETURN(object, result) \
    V_CHECK_RETURN(IsValid(object), result);

