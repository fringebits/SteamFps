// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include <GameFramework/GameSession.h>
#include "SteamFpsGameSession.generated.h"

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