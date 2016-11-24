// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#include "SteamFps.h"
#include "Screen.h"

USteamFpsScreenWidget::USteamFpsScreenWidget(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    V_TRACE_MARKER();
}

void USteamFpsScreenWidget::SetActive(bool flag)
{
    SetVisibility(flag ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}