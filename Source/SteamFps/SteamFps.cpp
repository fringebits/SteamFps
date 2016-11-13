// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SteamFps.h"


IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SteamFps, "SteamFps" );

#if PSEUDO_CODE

// GameInstance
void StateTransition(newState)
{
    if (currentState == newState)
    {
        return;
    }

    HideWidget(currentState);

    switch(currentState)
    {
    Playing: 
        DestroySessionCaller();
    }

    currentState = newState;

    ShowWidget(newState);
}

void DestroySessionCaller()
{
    auto pc = GetPlayerController(0);
    Online::DestroySession(pc);
}

// @29:34 -- Host a Game Session (will be called from MM widget button)
void HostGameEvent()
{
    auto pc = GetPlayerController(0);
    ShowLoadScreen();
    auto ret Online::CreateSession(pc, 2, m_useLan);
    if (SUCCESS(ret))
    {
        OpenLevel(MP_Level1, bAbsoluteTrue, "listen");
    }
    else
    {
        DisplayError("Failed to load level"); // @32:45
    }
}

void DisplayError(string)
{
    StateTransition(ErrorState);
    DestroySessionCaller();

    CreateWidgetIfNeeded(ErrorDialog);  // 34
}

void ShowMainMenu()
{
    if (IsCurrentState(Playing))
    {
        OpenLevel(MainMenuLevel);
    }

    StateTransition(MainMenu);

    auto widget = GetWidget(MainMenu);

    SetInputModeUIOnly(widget);
    ShowWidget(widegt);
}

void ShowServerList()
{
    StateTransition(ServerList);

}

void OnPlayButtonClicked()
{
    HostGameEvent();
}

void OnFindGamesBUttonClicked()
{
    ShowServerList();
}

void OnQuitBUttonClicked()
{
    Unreal::QuitGame();
}

void OnToggleLanClicked()
{
    m_isLanEnabled = !m_isLanEnabled;
}

#endif