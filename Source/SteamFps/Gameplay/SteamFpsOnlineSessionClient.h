// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OnlineSessionClient.h"
#include "SteamFpsOnlineSessionClient.generated.h"

UCLASS(Config = Game)
class UShooterOnlineSessionClient : public UOnlineSessionClient
{
	GENERATED_BODY()

public:
	/** Ctor */
	UShooterOnlineSessionClient();

	virtual void OnSessionUserInviteAccepted(
		const bool							bWasSuccess,
		const int32							ControllerId,
		TSharedPtr< const FUniqueNetId >	UserId,
		const FOnlineSessionSearchResult &	InviteResult
	) override;

};
