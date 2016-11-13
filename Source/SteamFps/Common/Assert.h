// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

namespace V1Asserts
{
    static auto const MESSAGE_BOX_TITLE_ASSERT = "V1 Assert";
}

#define X_MESSAGE_BOX_OK(message, title) V1MessageBox::Open(EAppMsgType::Ok, message, title)

#if !UE_BUILD_SHIPPING
// standard assert
#define V_ASSERT(condition, format, ...) \
{ \
    if(!(condition)) \
    { \
        V_ERROR_IF_S((condition) == false, AssertLog, format, ##__VA_ARGS__); \
        FString const message(X_FORMAT_LOG_OUTPUT_BRIEF_CONDITIONAL(condition ## (failed), format, ##__VA_ARGS__)); \
        X_MESSAGE_BOX_OK(message, V1Asserts::MESSAGE_BOX_TITLE_ASSERT); \
        check(condition) \
    } \
}
#else
#define V_ASSERT(condition, format, ...) do{}while(false);
#endif

#if !UE_BUILD_SHIPPING
// break if code path is executed
#define V_NO_ENTRY(format, ...) \
{ \
    V_ERROR_S(AssertLog, format, ##__VA_ARGS__); \
    FString const message = FString::Printf(TEXT("Invalid code path entry.\n%s"), *X_FORMAT_LOG_OUTPUT_NU(format, ##__VA_ARGS__)); \
    X_MESSAGE_BOX_OK(message, V1Asserts::MESSAGE_BOX_TITLE_ASSERT); \
    checkNoEntry(); \
}
#else
#define V_NO_ENTRY(format, ...) do{}while(false);
#endif

#if !UE_BUILD_SHIPPING
// break if code path is executed more than once in process lifetime
#define V_NO_REENTRY(format, ...) \
{ \
    V_ERROR_S(AssertLog, format, ##__VA_ARGS__); \
    FString const message = FString::Printf(TEXT("Invalid code path re-entry.\n%s"), *X_FORMAT_LOG_OUTPUT_NU(format, ##__VA_ARGS__)); \
    X_MESSAGE_BOX_OK(message, V1Asserts::MESSAGE_BOX_TITLE_ASSERT); \
    checkNoReentry(); \
}
#else
#define V_NO_REENTRY(message, ...) do{}while(false);
#endif


//#define V_ASSERT_RETURN(message, ...)