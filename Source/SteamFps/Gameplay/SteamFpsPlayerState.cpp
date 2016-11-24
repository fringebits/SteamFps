// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "SteamFpsPlayerController.h"
#include "SteamFpsPlayerState.h"

void ASteamFpsPlayerState::ClientInitialize(AController* controller)
{
    V_TRACE_MARKER();
    Super::ClientInitialize(controller);

    auto ctrl = Cast<ASteamFpsPlayerController>(controller);
    V_CHECK_VALID(ctrl);
}
