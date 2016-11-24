// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#pragma once

#include "Common/StateMachine.h"
#include "SteamFpsGameActor.generated.h"

UENUM()
enum class eGameState : uint8
{
    Startup
    ,MainMenu
    ,Playing
    ,ErrorDialog
    ,ServerList
    ,LoadingScreen
    ,EndOfMatch
    ,Unknown
    ,kCount          // Number of game states.
};

USTRUCT()
struct FGameStateStruct
{
    GENERATED_BODY()

    class UWidget* Widget;
};

UCLASS()
class STEAMFPS_API ASteamFpsGameActor 
    : public AActor
    , public SimpleMachine<eGameState>

{
	GENERATED_BODY()

public:
    ASteamFpsGameActor();

    void Tick(float dT) override;

    void OpenLevel(FName levelName, bool absolute = true, FString options = FString(TEXT("")));

protected:
    bool ServerTravel(const FString& url, bool absolute, bool shouldSkipGameNotify);

    void CreateOnlineSession();

    void DestroySession(APlayerController* playerController);

    void ErrorHandler();

    void CreateWidgets();

    void RefreshServerList();

protected:
    eGameState GetNextState(const eGameState cs, const eGameState ds) const override { return ds; }

    virtual void TickState(const eGameState gs, float dT) override;

    virtual void OnEnterState(const eGameState gs) override;

    virtual void OnExitState(const eGameState gs) override;

    virtual FString GetStateString(eGameState gs) const override { return V_FORMAT_ENUM(eGameState, gs); }

protected:
    void ShowWidget(eGameState gs, bool showOrHide);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool m_enableLan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameStateStruct> m_stateData;
};
