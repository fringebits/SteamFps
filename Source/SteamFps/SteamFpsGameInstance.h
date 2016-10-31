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
    Unknown
};

UCLASS()
class STEAMFPS_API USteamFpsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    UFUNCTION()
    bool IsCurrentState(eGameState gs)
    {
        return gs == m_currentState;
    }

    UFUNCTION()
    void StateTransition(eGameState gs)
    {
        if (IsCurrentState(gs))
        {
            // PrintToScreen("Error, trying to leaving curState.", Red, 5.0s);
            return;
        }

        // Handle exiting existing states
        //switch(m_currentState)
        //{
        //}

        // PrintToScreen("Setting new state {gs}", White, 5.0f);
        m_currentState = gs;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool m_isLanEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    eGameState m_currentState;

    //UPROPERTY()
    //    class UWidget* m_widgetMainMenu;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite)
    //    class UWidget* m_widgetServerList;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite)
    //    class UWidget* m_widgetLoading;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite)
    //    class UWidget* m_widetErrorDialog;
};
