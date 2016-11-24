// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#pragma once

#include <Engine/GameInstance.h>
#include "SteamFpsGameActor.h"
#include "SteamFpsGameInstance.generated.h"

UCLASS()
class STEAMFPS_API USteamFpsGameInstance 
    : public UGameInstance

{
	GENERATED_BODY()

public:
    USteamFpsGameInstance();
    ~USteamFpsGameInstance();

    static USteamFpsGameInstance* GetInstance();
    static ASteamFpsGameActor* GetActorInstance();

    virtual void Init() override;
    virtual void Shutdown() override;

protected:
    ASteamFpsGameActor* m_gameActor;

    void ShowWidget(eGameState gs, bool showOrHide);
};
