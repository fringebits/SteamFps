// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.

#pragma once

#include <Engine/GameInstance.h>
#include "SteamFpsGameInstance.generated.h"

UENUM(BlueprintType)
enum class eGameState : uint8
{
    Startup = 0,
    MainMenu,
    Playing,
    ErrorDialog,
    ServerList,
    Unknown,
    LoadingScreen,
    kCount
};

USTRUCT()
struct FGameStateStruct
{
    GENERATED_BODY()

    class UWidget* Widget;
};

UCLASS()
class STEAMFPS_API USteamFpsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    USteamFpsGameInstance();
    ~USteamFpsGameInstance();

    static USteamFpsGameInstance* GetInstance();

    void OpenLevel(FName levelName, bool absolute = true, FString options = FString(TEXT("")));

    UFUNCTION()
    bool IsInState(eGameState gs) const { return gs == m_currentState; }

    UFUNCTION()
    void StateTransition(eGameState gs);

    UFUNCTION(BlueprintCallable, Category="Menus")
    void ShowMainMenu();

protected:
    void ShowLoadScreen();
    void HostGameEvent();
    void DestroySession(APlayerController* playerController);
    bool ServerTravel(const FString& url, bool absolute, bool shouldSkipGameNotify);
    void ErrorHandler();

    void CreateWidgets();

    void OnEnterGameState(const eGameState gs);
    void OnExitGameState(const eGameState gs);

    void RefreshServerList();

protected:
    void ShowWidget(eGameState gs, bool showOrHide);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool m_enableLan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    eGameState m_currentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameStateStruct> m_stateData;

    // Need array of available sessions
    // TArray<BlueprintSessionResult> m_findServerResults;
};
