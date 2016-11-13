// Copyright (C) 2016 V1 Interactive Inc. - All Rights Reserved.
#pragma once

// Note:  MAJOR, MINOR numbers can be edited.  
// CHANGE, BUILD, BRANCH will be modified by the build machine.
#define STEAMFPS_VERSION_NUMBER "0.1.0.0" // Major, Minor, Change, Build
#define STEAMFPS_VERSION_BRANCH "master" // Branch of this build (default is "DEV") [unknown]

inline FString GetProjectVersionString()
{
    auto result = FString::Printf(
        TEXT("%s-%s"),
        TEXT(STEAMFPS_VERSION_NUMBER),
        TEXT(STEAMFPS_VERSION_BRANCH));
    return result;
}
