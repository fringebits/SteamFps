// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include <GameFramework/GameSession.h>
#include "SteamFpsGameSession.generated.h"

// AGameSession only exists on the server when running an online game.
// https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/AGameSession/index.html

UCLASS()
class ASteamFpsGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	ASteamFpsGameSession();

    virtual void BeginPlay() override;

    void CreateOnlineSession();

private:
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

    bool m_isLanMatch;
};