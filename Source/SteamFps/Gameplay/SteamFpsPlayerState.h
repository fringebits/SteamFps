// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#pragma once

#include "GameFramework/PlayerState.h"
#include "SteamFpsPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class STEAMFPS_API ASteamFpsPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

protected:
    /** Called by Controller when its PlayerState is initially replicated. */
    virtual void ClientInitialize(class AController* C);
};
