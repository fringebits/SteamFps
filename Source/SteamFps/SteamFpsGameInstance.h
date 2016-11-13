// Fill out your copyright notice in the Description page of Project Settings.

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

    UFUNCTION()
    bool IsInState(eGameState gs) const { return gs == m_currentState; }

    UFUNCTION()
    void StateTransition(eGameState gs);

    UFUNCTION(BlueprintCallable, Category="Menus")
    void ShowMainMenu();

protected:
    void ShowLoadScreen();
    void HostGameEvent();
    void CreateSession();
    void DestroySession(APlayerController* playerController);
    bool ServerTravel(const FString& url, bool absolute, bool shouldSkipGameNotify);
    void ErrorHandler();

    void CreateWidgets();

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
