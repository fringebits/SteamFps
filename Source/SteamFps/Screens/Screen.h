// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include <Blueprint/UserWidget.h>
#include "Screen.generated.h"

UCLASS()
class USteamFpsScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    USteamFpsScreenWidget(const FObjectInitializer& objectInitializer);

    void SetActive(bool flag);
};