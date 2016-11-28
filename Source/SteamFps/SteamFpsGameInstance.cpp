// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SteamFps.h"
#include <Blueprint/UserWidget.h>
#include <OnlineSessionInterface.h>
#include <Online.h>

#include "SteamFpsGameInstance.h"

#include "UI/Menu/ShooterMainMenu.h"
#include "UI/Menu/ShooterWelcomeMenu.h"
#include "UI/Menu/ShooterMessageMenu.h"
//#include "UI/Menu/ShooterGameLoadingScreen.h"

#include "OnlineKeyValuePair.h"
#include "UI/Style/ShooterStyle.h"
#include "UI/Style/ShooterMenuItemWidgetStyle.h"
#include "SteamFpsGameViewportClient.h"
#include "Gameplay/SteamFpsPlayerState.h"
#include "SteamFpsGameSession.h"
#include "Gameplay/SteamFpsOnlineSessionClient.h"

namespace
{
    std::list<USteamFpsGameInstance*> s_instanceList;
}

USteamFpsGameInstance::USteamFpsGameInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    //, m_gameActor(nullptr)
    , m_isOnline(true) // Default to online
    , bIsLicensed(true) // Default to licensed (should have been checked by OS on boot)
{
    CurrentState = ShooterGameInstanceState::None;
    s_instanceList.emplace_back(this);
}

USteamFpsGameInstance::~USteamFpsGameInstance()
{
    s_instanceList.remove(this);
}

USteamFpsGameInstance* USteamFpsGameInstance::GetInstance()
{
    return !s_instanceList.empty() ? s_instanceList.back() : nullptr;
}

ASteamFpsGameActor* USteamFpsGameInstance::GetActorInstance()
{
    auto gi = GetInstance();
    return nullptr;
    //return (gi != nullptr) ? gi->m_gameActor : nullptr;
}

//void USteamFpsGameInstance::Init()
//{
//    V_TRACE_MARKER();
//
//    auto world = GetWorld();
//    V_CHECK_VALID(world);
//
//    m_gameActor = world->SpawnActor<ASteamFpsGameActor>();
//}

//void USteamFpsGameInstance::Shutdown()
//{
//    V_TRACE_MARKER();
//
//    V_CHECK_VALID(m_gameActor);
//    m_gameActor->Destroy();
//}

void USteamFpsGameInstance::SetIsOnline(bool flag)
{
    m_isOnline = flag;
    auto oss = IOnlineSubsystem::Get();

    V_CHECK_NULL(oss);

    for (auto ii = 0; ii < LocalPlayers.Num(); ++ii)
    {
        auto localPlayer = LocalPlayers[ii];

        auto pid = localPlayer->GetPreferredUniqueNetId();
        if (pid.IsValid())
        {
            oss->SetUsingMultiplayerFeatures(*pid, m_isOnline);
        }
    }
}

void SShooterWaitDialog::Construct(const FArguments& InArgs)
{
    const FShooterMenuItemStyle* ItemStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterMenuItemStyle>("DefaultShooterMenuItemStyle");
    const FButtonStyle* ButtonStyle = &FShooterStyle::Get().GetWidgetStyle<FButtonStyle>("DefaultShooterButtonStyle");
    ChildSlot
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(20.0f)
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        [
            SNew(SBorder)
            .Padding(50.0f)
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        .BorderImage(&ItemStyle->BackgroundBrush)
        .BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
        [
            SNew(STextBlock)
            .TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
        .ColorAndOpacity(this, &SShooterWaitDialog::GetTextColor)
        .Text(InArgs._MessageText)
        .WrapTextAt(500.0f)
        ]
        ]
        ];

    //Setup a curve
    const float StartDelay = 0.0f;
    const float SecondDelay = 0.0f;
    const float AnimDuration = 2.0f;

    WidgetAnimation = FCurveSequence();
    TextColorCurve = WidgetAnimation.AddCurve(StartDelay + SecondDelay, AnimDuration, ECurveEaseFunction::QuadInOut);
    WidgetAnimation.Play(this->AsShared(), true);
}

FSlateColor SShooterWaitDialog::GetTextColor() const
{
    //instead of going from black -> white, go from white -> grey.
    float fAlpha = 1.0f - TextColorCurve.GetLerp();
    fAlpha = fAlpha * 0.5f + 0.5f;
    return FLinearColor(FColor(155, 164, 182, FMath::Clamp((int32)(fAlpha * 255.0f), 0, 255)));
}

namespace ShooterGameInstanceState
{
    const FName None = FName(TEXT("None"));
    const FName PendingInvite = FName(TEXT("PendingInvite"));
    const FName WelcomeScreen = FName(TEXT("WelcomeScreen"));
    const FName MainMenu = FName(TEXT("MainMenu"));
    const FName MessageMenu = FName(TEXT("MessageMenu"));
    const FName Playing = FName(TEXT("Playing"));
}


void USteamFpsGameInstance::Init()
{
    Super::Init();

    IgnorePairingChangeForControllerId = -1;
    CurrentConnectionStatus = EOnlineServerConnectionStatus::Connected;

    // game requires the ability to ID users.
    const auto OnlineSub = IOnlineSubsystem::Get();
    check(OnlineSub);
    const auto IdentityInterface = OnlineSub->GetIdentityInterface();
    check(IdentityInterface.IsValid());

    const auto SessionInterface = OnlineSub->GetSessionInterface();
    check(SessionInterface.IsValid());

    // bind any OSS delegates we needs to handle
    for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
    {
        IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(i, FOnLoginStatusChangedDelegate::CreateUObject(this, &USteamFpsGameInstance::HandleUserLoginChanged));
    }

    IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(FOnControllerPairingChangedDelegate::CreateUObject(this, &USteamFpsGameInstance::HandleControllerPairingChanged));

    FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &USteamFpsGameInstance::HandleAppWillDeactivate);

    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &USteamFpsGameInstance::HandleAppSuspend);
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &USteamFpsGameInstance::HandleAppResume);

    FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &USteamFpsGameInstance::HandleSafeFrameChanged);
    FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &USteamFpsGameInstance::HandleControllerConnectionChange);
    FCoreDelegates::ApplicationLicenseChange.AddUObject(this, &USteamFpsGameInstance::HandleAppLicenseUpdate);

    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USteamFpsGameInstance::OnPreLoadMap);
    FCoreUObjectDelegates::PostLoadMap.AddUObject(this, &USteamFpsGameInstance::OnPostLoadMap);

    FCoreUObjectDelegates::PostDemoPlay.AddUObject(this, &USteamFpsGameInstance::OnPostDemoPlay);

    bPendingEnableSplitscreen = false;

    OnlineSub->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &USteamFpsGameInstance::HandleNetworkConnectionStatusChanged));

    SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &USteamFpsGameInstance::HandleSessionFailure));

    OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &USteamFpsGameInstance::OnEndSessionComplete);

    // Register delegate for ticker callback
    TickDelegate = FTickerDelegate::CreateUObject(this, &USteamFpsGameInstance::Tick);
    TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
}

void USteamFpsGameInstance::Shutdown()
{
    Super::Shutdown();

    // Unregister ticker delegate
    FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void USteamFpsGameInstance::HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus)
{
    UE_LOG(LogOnlineGame, Warning, TEXT("USteamFpsGameInstance::HandleNetworkConnectionStatusChanged: %s"), EOnlineServerConnectionStatus::ToString(ConnectionStatus));

#if SHOOTER_CONSOLE_UI
    // If we are disconnected from server, and not currently at (or heading to) the welcome screen
    // then display a message on consoles
    if (bIsOnline &&
        PendingState != ShooterGameInstanceState::WelcomeScreen &&
        CurrentState != ShooterGameInstanceState::WelcomeScreen &&
        ConnectionStatus != EOnlineServerConnectionStatus::Connected)
    {
        UE_LOG(LogOnlineGame, Log, TEXT("USteamFpsGameInstance::HandleNetworkConnectionStatusChanged: Going to main menu"));

        // Display message on consoles
#if PLATFORM_XBOXONE
        const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to Xbox LIVE has been lost.");
#elif PLATFORM_PS4
        const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to \"PSN\" has been lost.");
#else
        const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection has been lost.");
#endif
        const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

        ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), ShooterGameInstanceState::MainMenu);
    }

    CurrentConnectionStatus = ConnectionStatus;
#endif
}

void USteamFpsGameInstance::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
    UE_LOG(LogOnlineGame, Warning, TEXT("USteamFpsGameInstance::HandleSessionFailure: %u"), (uint32)FailureType);

#if SHOOTER_CONSOLE_UI
    // If we are not currently at (or heading to) the welcome screen then display a message on consoles
    if (bIsOnline &&
        PendingState != ShooterGameInstanceState::WelcomeScreen &&
        CurrentState != ShooterGameInstanceState::WelcomeScreen)
    {
        UE_LOG(LogOnlineGame, Log, TEXT("USteamFpsGameInstance::HandleSessionFailure: Going to main menu"));

        // Display message on consoles
#if PLATFORM_XBOXONE
        const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to Xbox LIVE has been lost.");
#elif PLATFORM_PS4
        const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to PSN has been lost.");
#else
        const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection has been lost.");
#endif
        const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

        ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), ShooterGameInstanceState::MainMenu);
    }
#endif
}

void USteamFpsGameInstance::OnPreLoadMap(const FString& MapName)
{
    if (bPendingEnableSplitscreen)
    {
        // Allow splitscreen
        GetGameViewportClient()->SetDisableSplitscreenOverride(false);

        bPendingEnableSplitscreen = false;
    }
}

void USteamFpsGameInstance::OnPostLoadMap()
{
    // Make sure we hide the loading screen when the level is done loading
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

    if (ShooterViewport != NULL)
    {
        ShooterViewport->HideLoadingScreen();
    }
}

void USteamFpsGameInstance::OnUserCanPlayInvite(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
    CleanupOnlinePrivilegeTask();
    if (WelcomeMenuUI.IsValid())
    {
        WelcomeMenuUI->LockControls(false);
    }

    if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
    {
        if (UserId == *PendingInvite.UserId)
        {
            PendingInvite.bPrivilegesCheckedAndAllowed = true;
        }
    }
    else
    {
        DisplayOnlinePrivilegeFailureDialogs(UserId, Privilege, PrivilegeResults);
        GotoState(ShooterGameInstanceState::WelcomeScreen);
    }
}

void USteamFpsGameInstance::OnPostDemoPlay()
{
    GotoState(ShooterGameInstanceState::Playing);
}

void USteamFpsGameInstance::HandleDemoPlaybackFailure(EDemoPlayFailure::Type FailureType, const FString& ErrorString)
{
    ShowMessageThenGotoState(FText::Format(NSLOCTEXT("USteamFpsGameInstance", "DemoPlaybackFailedFmt", "Demo playback failed: {0}"), FText::FromString(ErrorString)), NSLOCTEXT("DialogButtons", "OKAY", "OK"), FText::GetEmpty(), ShooterGameInstanceState::MainMenu);
}

void USteamFpsGameInstance::StartGameInstance()
{
#if PLATFORM_PS4 == 0
    TCHAR Parm[4096] = TEXT("");

    const TCHAR* Cmd = FCommandLine::Get();

    // Catch the case where we want to override the map name on startup (used for connecting to other MP instances)
    if (FParse::Token(Cmd, Parm, ARRAY_COUNT(Parm), 0) && Parm[0] != '-')
    {
        // if we're 'overriding' with the default map anyway, don't set a bogus 'playing' state.
        if (!MainMenuMap.Contains(Parm))
        {
            FURL DefaultURL;
            DefaultURL.LoadURLConfig(TEXT("DefaultPlayer"), GGameIni);

            FURL URL(&DefaultURL, Parm, TRAVEL_Partial);

            if (URL.Valid)
            {
                UEngine* const Engine = GetEngine();

                FString Error;

                const EBrowseReturnVal::Type BrowseRet = Engine->Browse(*WorldContext, URL, Error);

                if (BrowseRet == EBrowseReturnVal::Success)
                {
                    // Success, we loaded the map, go directly to playing state
                    GotoState(ShooterGameInstanceState::Playing);
                    return;
                }
                else if (BrowseRet == EBrowseReturnVal::Pending)
                {
                    // Assume network connection
                    LoadFrontEndMap(MainMenuMap);
                    AddNetworkFailureHandlers();
                    ShowLoadingScreen();
                    GotoState(ShooterGameInstanceState::Playing);
                    return;
                }
            }
        }
    }
#endif

    GotoInitialState();
}

FName USteamFpsGameInstance::GetInitialState()
{
#if SHOOTER_CONSOLE_UI	
    // Start in the welcome screen state on consoles
    return ShooterGameInstanceState::WelcomeScreen;
#else
    // On PC, go directly to the main menu
    return ShooterGameInstanceState::MainMenu;
#endif
}

void USteamFpsGameInstance::GotoInitialState()
{
    GotoState(GetInitialState());
}

void USteamFpsGameInstance::ShowMessageThenGotoState(const FText& Message, const FText& OKButtonString, const FText& CancelButtonString, const FName& NewState, const bool OverrideExisting, TWeakObjectPtr< ULocalPlayer > PlayerOwner)
{
    UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Message: %s, NewState: %s"), *Message.ToString(), *NewState.ToString());

    const bool bAtWelcomeScreen = PendingState == ShooterGameInstanceState::WelcomeScreen || CurrentState == ShooterGameInstanceState::WelcomeScreen;

    // Never override the welcome screen
    if (bAtWelcomeScreen)
    {
        UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (at welcome screen)."));
        return;
    }

    const bool bAlreadyAtMessageMenu = PendingState == ShooterGameInstanceState::MessageMenu || CurrentState == ShooterGameInstanceState::MessageMenu;
    const bool bAlreadyAtDestState = PendingState == NewState || CurrentState == NewState;

    // If we are already going to the message menu, don't override unless asked to
    if (bAlreadyAtMessageMenu && PendingMessage.NextState == NewState && !OverrideExisting)
    {
        UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 1)."));
        return;
    }

    // If we are already going to the message menu, and the next dest is welcome screen, don't override
    if (bAlreadyAtMessageMenu && PendingMessage.NextState == ShooterGameInstanceState::WelcomeScreen)
    {
        UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 2)."));
        return;
    }

    // If we are already at the dest state, don't override unless asked
    if (bAlreadyAtDestState && !OverrideExisting)
    {
        UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 3)"));
        return;
    }

    PendingMessage.DisplayString = Message;
    PendingMessage.OKButtonString = OKButtonString;
    PendingMessage.CancelButtonString = CancelButtonString;
    PendingMessage.NextState = NewState;
    PendingMessage.PlayerOwner = PlayerOwner;

    if (CurrentState == ShooterGameInstanceState::MessageMenu)
    {
        UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Forcing new message"));
        EndMessageMenuState();
        BeginMessageMenuState();
    }
    else
    {
        GotoState(ShooterGameInstanceState::MessageMenu);
    }
}

void USteamFpsGameInstance::ShowLoadingScreen()
{
    // This can be confusing, so here is what is happening:
    //	For LoadMap, we use the IShooterGameLoadingScreenModule interface to show the load screen
    //  This is necessary since this is a blocking call, and our viewport loading screen won't get updated.
    //  We can't use IShooterGameLoadingScreenModule for seamless travel though
    //  In this case, we just add a widget to the viewport, and have it update on the main thread
    //  To simplify things, we just do both, and you can't tell, one will cover the other if they both show at the same time


    //IShooterGameLoadingScreenModule* const LoadingScreenModule = FModuleManager::LoadModulePtr<IShooterGameLoadingScreenModule>("ShooterGameLoadingScreen");
    //if (LoadingScreenModule != nullptr)
    //{
    //    LoadingScreenModule->StartInGameLoadingScreen();
    //}

    //UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

    //if (ShooterViewport != NULL)
    //{
    //    ShooterViewport->ShowLoadingScreen();
    //}
}

bool USteamFpsGameInstance::LoadFrontEndMap(const FString& MapName)
{
    bool bSuccess = true;

    // if already loaded, do nothing
    UWorld* const World = GetWorld();
    if (World)
    {
        FString const CurrentMapName = *World->PersistentLevel->GetOutermost()->GetName();
        //if (MapName.Find(TEXT("Highrise")) != -1)
        if (CurrentMapName == MapName)
        {
            return bSuccess;
        }
    }

    FString Error;
    EBrowseReturnVal::Type BrowseRet = EBrowseReturnVal::Failure;
    FURL URL(
        *FString::Printf(TEXT("%s"), *MapName)
    );

    if (URL.Valid && !HasAnyFlags(RF_ClassDefaultObject)) //CastChecked<UEngine>() will fail if using Default__ShooterGameInstance, so make sure that we're not default
    {
        BrowseRet = GetEngine()->Browse(*WorldContext, URL, Error);

        // Handle failure.
        if (BrowseRet != EBrowseReturnVal::Success)
        {
            UE_LOG(LogLoad, Fatal, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), *MapName, *Error));
            bSuccess = false;
        }
    }
    return bSuccess;
}

ASteamFpsGameSession* USteamFpsGameInstance::GetGameSession() const
{
    UWorld* const World = GetWorld();
    if (World)
    {
        AGameMode* const Game = World->GetAuthGameMode();
        if (Game)
        {
            return Cast<ASteamFpsGameSession>(Game->GameSession);
        }
    }

    return nullptr;
}

void USteamFpsGameInstance::TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
    // BUGBUG: not supported
    //ASteamFpsPlayerController_Menu* const FirstPC = Cast<ASteamFpsPlayerController_Menu>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    //if (FirstPC != nullptr)
    //{
    //    FText ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join Session failed.");
    //    if (ReasonString.IsEmpty() == false)
    //    {
    //        ReturnReason = FText::Format(NSLOCTEXT("NetworkErrors", "JoinSessionFailedReasonFmt", "Join Session failed. {0}"), FText::FromString(ReasonString));
    //    }

    //    FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
    //    ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
    //}
}

void USteamFpsGameInstance::ShowMessageThenGoMain(const FText& Message, const FText& OKButtonString, const FText& CancelButtonString)
{
    ShowMessageThenGotoState(Message, OKButtonString, CancelButtonString, ShooterGameInstanceState::MainMenu);
}

void USteamFpsGameInstance::SetPendingInvite(const FShooterPendingInvite& InPendingInvite)
{
    PendingInvite = InPendingInvite;
}

void USteamFpsGameInstance::GotoState(FName NewState)
{
    UE_LOG(LogOnline, Log, TEXT("GotoState: NewState: %s"), *NewState.ToString());

    PendingState = NewState;
}

void USteamFpsGameInstance::MaybeChangeState()
{
    if ((PendingState != CurrentState) && (PendingState != ShooterGameInstanceState::None))
    {
        FName const OldState = CurrentState;

        // end current state
        EndCurrentState(PendingState);

        // begin new state
        BeginNewState(PendingState, OldState);

        // clear pending change
        PendingState = ShooterGameInstanceState::None;
    }
}

void USteamFpsGameInstance::EndCurrentState(FName NextState)
{
    // per-state custom ending code here
    if (CurrentState == ShooterGameInstanceState::PendingInvite)
    {
        EndPendingInviteState();
    }
    else if (CurrentState == ShooterGameInstanceState::WelcomeScreen)
    {
        EndWelcomeScreenState();
    }
    else if (CurrentState == ShooterGameInstanceState::MainMenu)
    {
        EndMainMenuState();
    }
    else if (CurrentState == ShooterGameInstanceState::MessageMenu)
    {
        EndMessageMenuState();
    }
    else if (CurrentState == ShooterGameInstanceState::Playing)
    {
        EndPlayingState();
    }

    CurrentState = ShooterGameInstanceState::None;
}

void USteamFpsGameInstance::BeginNewState(FName NewState, FName PrevState)
{
    // per-state custom starting code here

    if (NewState == ShooterGameInstanceState::PendingInvite)
    {
        BeginPendingInviteState();
    }
    else if (NewState == ShooterGameInstanceState::WelcomeScreen)
    {
        BeginWelcomeScreenState();
    }
    else if (NewState == ShooterGameInstanceState::MainMenu)
    {
        BeginMainMenuState();
    }
    else if (NewState == ShooterGameInstanceState::MessageMenu)
    {
        BeginMessageMenuState();
    }
    else if (NewState == ShooterGameInstanceState::Playing)
    {
        BeginPlayingState();
    }

    CurrentState = NewState;
}

void USteamFpsGameInstance::BeginPendingInviteState()
{
    if (LoadFrontEndMap(MainMenuMap))
    {
        StartOnlinePrivilegeTask(IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(this, &USteamFpsGameInstance::OnUserCanPlayInvite), EUserPrivileges::CanPlayOnline, PendingInvite.UserId);
    }
    else
    {
        GotoState(ShooterGameInstanceState::WelcomeScreen);
    }
}

void USteamFpsGameInstance::EndPendingInviteState()
{
    // cleanup in case the state changed before the pending invite was handled.
    CleanupOnlinePrivilegeTask();
}

void USteamFpsGameInstance::BeginWelcomeScreenState()
{
    //this must come before split screen player removal so that the OSS sets all players to not using online features.
    SetIsOnline(false);

    // Remove any possible splitscren players
    RemoveSplitScreenPlayers();

    LoadFrontEndMap(WelcomeScreenMap);

    ULocalPlayer* const LocalPlayer = GetFirstGamePlayer();
    LocalPlayer->SetCachedUniqueNetId(nullptr);
    check(!WelcomeMenuUI.IsValid());
    WelcomeMenuUI = MakeShareable(new FShooterWelcomeMenu);
    WelcomeMenuUI->Construct(this);
    WelcomeMenuUI->AddToGameViewport();

    // Disallow splitscreen (we will allow while in the playing state)
    GetGameViewportClient()->SetDisableSplitscreenOverride(true);
}

void USteamFpsGameInstance::EndWelcomeScreenState()
{
    if (WelcomeMenuUI.IsValid())
    {
        WelcomeMenuUI->RemoveFromGameViewport();
        WelcomeMenuUI = nullptr;
    }
}

void USteamFpsGameInstance::SetPresenceForLocalPlayers(const FVariantData& PresenceData)
{
    const auto Presence = Online::GetPresenceInterface();
    if (Presence.IsValid())
    {
        for (int i = 0; i < LocalPlayers.Num(); ++i)
        {
            const TSharedPtr<const FUniqueNetId> UserId = LocalPlayers[i]->GetPreferredUniqueNetId();

            if (UserId.IsValid())
            {
                FOnlineUserPresenceStatus PresenceStatus;
                PresenceStatus.Properties.Add(DefaultPresenceKey, PresenceData);

                Presence->SetPresence(*UserId, PresenceStatus);
            }
        }
    }
}

void USteamFpsGameInstance::BeginMainMenuState()
{
    // Make sure we're not showing the loadscreen
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

    if (ShooterViewport != NULL)
    {
        ShooterViewport->HideLoadingScreen();
    }

    SetIsOnline(false);

    // Disallow splitscreen
    GetGameViewportClient()->SetDisableSplitscreenOverride(true);

    // Remove any possible splitscren players
    RemoveSplitScreenPlayers();

    // Set presence to menu state for the owning player
    SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

    // load startup map
    LoadFrontEndMap(MainMenuMap);

    // player 0 gets to own the UI
    ULocalPlayer* const Player = GetFirstGamePlayer();

    MainMenuUI = MakeShareable(new FShooterMainMenu());
    MainMenuUI->Construct(this, Player);
    MainMenuUI->AddMenuToGameViewport();

#if !SHOOTER_CONSOLE_UI
    // The cached unique net ID is usually set on the welcome screen, but there isn't
    // one on PC/Mac, so do it here.
    if (Player != nullptr)
    {
        Player->SetControllerId(0);
        Player->SetCachedUniqueNetId(Player->GetUniqueNetIdFromCachedControllerId());
    }
#endif

    RemoveNetworkFailureHandlers();
}

void USteamFpsGameInstance::EndMainMenuState()
{
    if (MainMenuUI.IsValid())
    {
        MainMenuUI->RemoveMenuFromGameViewport();
        MainMenuUI = nullptr;
    }
}

void USteamFpsGameInstance::BeginMessageMenuState()
{
    if (PendingMessage.DisplayString.IsEmpty())
    {
        UE_LOG(LogOnlineGame, Warning, TEXT("USteamFpsGameInstance::BeginMessageMenuState: Display string is empty"));
        GotoInitialState();
        return;
    }

    // Make sure we're not showing the loadscreen
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

    if (ShooterViewport != NULL)
    {
        ShooterViewport->HideLoadingScreen();
    }

    check(!MessageMenuUI.IsValid());
    MessageMenuUI = MakeShareable(new FShooterMessageMenu);
    MessageMenuUI->Construct(this, PendingMessage.PlayerOwner, PendingMessage.DisplayString, PendingMessage.OKButtonString, PendingMessage.CancelButtonString, PendingMessage.NextState);

    PendingMessage.DisplayString = FText::GetEmpty();
}

void USteamFpsGameInstance::EndMessageMenuState()
{
    if (MessageMenuUI.IsValid())
    {
        MessageMenuUI->RemoveFromGameViewport();
        MessageMenuUI = nullptr;
    }
}

void USteamFpsGameInstance::BeginPlayingState()
{
    bPendingEnableSplitscreen = true;

    // Set presence for playing in a map
    SetPresenceForLocalPlayers(FVariantData(FString(TEXT("InGame"))));

    // Make sure viewport has focus
    FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void USteamFpsGameInstance::EndPlayingState()
{
    // Disallow splitscreen
    GetGameViewportClient()->SetDisableSplitscreenOverride(true);

    // Clear the players' presence information
    SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

    UWorld* const World = GetWorld();
    ASteamFpsGameState* const GameState = World != NULL ? World->GetGameState<ASteamFpsGameState>() : NULL;

    if (GameState)
    {
        // Send round end events for local players
        for (int i = 0; i < LocalPlayers.Num(); ++i)
        {
            auto ShooterPC = Cast<ASteamFpsPlayerController>(LocalPlayers[i]->PlayerController);
            if (ShooterPC)
            {
                // Assuming you can't win if you quit early
                ShooterPC->ClientSendRoundEndEvent(false, GameState->ElapsedTime);
            }
        }

        // Give the game state a chance to cleanup first
        GameState->RequestFinishAndExitToMainMenu();
    }
    else
    {
        // If there is no game state, make sure the session is in a good state
        CleanupSessionOnReturnToMenu();
    }
}

void USteamFpsGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogOnline, Log, TEXT("USteamFpsGameInstance::OnEndSessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
            Sessions->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegateHandle);
            Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
        }
    }

    // continue
    CleanupSessionOnReturnToMenu();
}

void USteamFpsGameInstance::CleanupSessionOnReturnToMenu()
{
    bool bPendingOnlineOp = false;

    // end online game and then destroy it
    IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
    IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

    if (Sessions.IsValid())
    {
        EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);
        UE_LOG(LogOnline, Log, TEXT("Session %s is '%s'"), *GameSessionName.ToString(), EOnlineSessionState::ToString(SessionState));

        if (EOnlineSessionState::InProgress == SessionState)
        {
            UE_LOG(LogOnline, Log, TEXT("Ending session %s on return to main menu"), *GameSessionName.ToString());
            OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
            Sessions->EndSession(GameSessionName);
            bPendingOnlineOp = true;
        }
        else if (EOnlineSessionState::Ending == SessionState)
        {
            UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to end on return to main menu"), *GameSessionName.ToString());
            OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
            bPendingOnlineOp = true;
        }
        else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
        {
            UE_LOG(LogOnline, Log, TEXT("Destroying session %s on return to main menu"), *GameSessionName.ToString());
            OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
            Sessions->DestroySession(GameSessionName);
            bPendingOnlineOp = true;
        }
        else if (EOnlineSessionState::Starting == SessionState)
        {
            UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to start, and then we will end it to return to main menu"), *GameSessionName.ToString());
            OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
            bPendingOnlineOp = true;
        }
    }

    if (!bPendingOnlineOp)
    {
        //GEngine->HandleDisconnect( GetWorld(), GetWorld()->GetNetDriver() );
    }
}

void USteamFpsGameInstance::LabelPlayerAsQuitter(ULocalPlayer* LocalPlayer) const
{
    ASteamFpsPlayerState* const PlayerState = LocalPlayer && LocalPlayer->PlayerController ? Cast<ASteamFpsPlayerState>(LocalPlayer->PlayerController->PlayerState) : nullptr;
    if (PlayerState)
    {
        PlayerState->SetQuitter(true);
    }
}

void USteamFpsGameInstance::RemoveNetworkFailureHandlers()
{
    // Remove the local session/travel failure bindings if they exist
    if (GEngine->OnTravelFailure().IsBoundToObject(this) == true)
    {
        GEngine->OnTravelFailure().Remove(TravelLocalSessionFailureDelegateHandle);
    }
}

void USteamFpsGameInstance::AddNetworkFailureHandlers()
{
    // Add network/travel error handlers (if they are not already there)
    if (GEngine->OnTravelFailure().IsBoundToObject(this) == false)
    {
        TravelLocalSessionFailureDelegateHandle = GEngine->OnTravelFailure().AddUObject(this, &USteamFpsGameInstance::TravelLocalSessionFailure);
    }
}

TSubclassOf<UOnlineSession> USteamFpsGameInstance::GetOnlineSessionClass()
{
    return UShooterOnlineSessionClient::StaticClass();
}

// starts playing a game as the host
bool USteamFpsGameInstance::HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL)
{
    if (!GetIsOnline())
    {
        //
        // Offline game, just go straight to map
        //

        ShowLoadingScreen();
        GotoState(ShooterGameInstanceState::Playing);

        // Travel to the specified match URL
        TravelURL = InTravelURL;
        GetWorld()->ServerTravel(TravelURL);
        return true;
    }

    //
    // Online game
    //

    ASteamFpsGameSession* const GameSession = GetGameSession();
    if (GameSession)
    {
        // add callback delegate for completion
        OnCreatePresenceSessionCompleteDelegateHandle = GameSession->OnCreatePresenceSessionComplete().AddUObject(this, &USteamFpsGameInstance::OnCreatePresenceSessionComplete);

        TravelURL = InTravelURL;
        bool const bIsLanMatch = InTravelURL.Contains(TEXT("?bIsLanMatch"));

        //determine the map name from the travelURL
        const FString& MapNameSubStr = "/Game/Maps/";
        const FString& ChoppedMapName = TravelURL.RightChop(MapNameSubStr.Len());
        const FString& MapName = ChoppedMapName.LeftChop(ChoppedMapName.Len() - ChoppedMapName.Find("?game"));

        if (GameSession->HostSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, GameType, MapName, bIsLanMatch, true, ASteamFpsGameSession::DEFAULT_NUM_PLAYERS))
        {
            // If any error occured in the above, pending state would be set
            if ((PendingState == CurrentState) || (PendingState == ShooterGameInstanceState::None))
            {
                // Go ahead and go into loading state now
                // If we fail, the delegate will handle showing the proper messaging and move to the correct state
                ShowLoadingScreen();
                GotoState(ShooterGameInstanceState::Playing);
                return true;
            }
        }
    }

    return false;
}

bool USteamFpsGameInstance::JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults)
{
    // needs to tear anything down based on current state?

    ASteamFpsGameSession* const GameSession = GetGameSession();
    if (GameSession)
    {
        AddNetworkFailureHandlers();

        OnJoinSessionCompleteDelegateHandle = GameSession->OnJoinSessionComplete().AddUObject(this, &USteamFpsGameInstance::OnJoinSessionComplete);
        if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SessionIndexInSearchResults))
        {
            // If any error occured in the above, pending state would be set
            if ((PendingState == CurrentState) || (PendingState == ShooterGameInstanceState::None))
            {
                // Go ahead and go into loading state now
                // If we fail, the delegate will handle showing the proper messaging and move to the correct state
                ShowLoadingScreen();
                GotoState(ShooterGameInstanceState::Playing);
                return true;
            }
        }
    }

    return false;
}

bool USteamFpsGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
    // needs to tear anything down based on current state?
    ASteamFpsGameSession* const GameSession = GetGameSession();
    if (GameSession)
    {
        AddNetworkFailureHandlers();

        OnJoinSessionCompleteDelegateHandle = GameSession->OnJoinSessionComplete().AddUObject(this, &USteamFpsGameInstance::OnJoinSessionComplete);
        if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SearchResult))
        {
            // If any error occured in the above, pending state would be set
            if ((PendingState == CurrentState) || (PendingState == ShooterGameInstanceState::None))
            {
                // Go ahead and go into loading state now
                // If we fail, the delegate will handle showing the proper messaging and move to the correct state
                ShowLoadingScreen();
                GotoState(ShooterGameInstanceState::Playing);
                return true;
            }
        }
    }

    return false;
}

bool USteamFpsGameInstance::PlayDemo(ULocalPlayer* LocalPlayer, const FString& DemoName)
{
    ShowLoadingScreen();

    // Play the demo
    PlayReplay(DemoName);

    return true;
}

/** Callback which is intended to be called upon finding sessions */
void USteamFpsGameInstance::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
    // unhook the delegate
    ASteamFpsGameSession* const GameSession = GetGameSession();
    if (GameSession)
    {
        GameSession->OnJoinSessionComplete().Remove(OnJoinSessionCompleteDelegateHandle);
    }

    // Add the splitscreen player if one exists
    if (Result == EOnJoinSessionCompleteResult::Success && LocalPlayers.Num() > 1)
    {
        auto Sessions = Online::GetSessionInterface();
        if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
        {
            Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), GameSessionName,
                FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &USteamFpsGameInstance::OnRegisterJoiningLocalPlayerComplete));
        }
    }
    else
    {
        // We either failed or there is only a single local user
        FinishJoinSession(Result);
    }
}

void USteamFpsGameInstance::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        FText ReturnReason;
        switch (Result)
        {
        case EOnJoinSessionCompleteResult::SessionIsFull:
            ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game is full.");
            break;
        case EOnJoinSessionCompleteResult::SessionDoesNotExist:
            ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game no longer exists.");
            break;
        default:
            ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.");
            break;
        }

        FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
        RemoveNetworkFailureHandlers();
        ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
        return;
    }

    InternalTravelToSession(GameSessionName);
}

void USteamFpsGameInstance::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
    FinishJoinSession(Result);
}

void USteamFpsGameInstance::InternalTravelToSession(const FName& SessionName)
{
    APlayerController * const PlayerController = GetFirstLocalPlayerController();

    if (PlayerController == nullptr)
    {
        FText ReturnReason = NSLOCTEXT("NetworkErrors", "InvalidPlayerController", "Invalid Player Controller");
        FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
        RemoveNetworkFailureHandlers();
        ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
        return;
    }

    // travel to session
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

    if (OnlineSub == nullptr)
    {
        FText ReturnReason = NSLOCTEXT("NetworkErrors", "OSSMissing", "OSS missing");
        FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
        RemoveNetworkFailureHandlers();
        ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
        return;
    }

    FString URL;
    IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

    if (!Sessions.IsValid() || !Sessions->GetResolvedConnectString(SessionName, URL))
    {
        FText FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.");
        FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
        ShowMessageThenGoMain(FailReason, OKButton, FText::GetEmpty());
        UE_LOG(LogOnlineGame, Warning, TEXT("Failed to travel to session upon joining it"));
        return;
    }

    PlayerController->ClientTravel(URL, TRAVEL_Absolute);
}

/** Callback which is intended to be called upon session creation */
void USteamFpsGameInstance::OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful)
{
    ASteamFpsGameSession* const GameSession = GetGameSession();
    if (GameSession)
    {
        GameSession->OnCreatePresenceSessionComplete().Remove(OnCreatePresenceSessionCompleteDelegateHandle);

        // Add the splitscreen player if one exists
        if (bWasSuccessful && LocalPlayers.Num() > 1)
        {
            auto Sessions = Online::GetSessionInterface();
            if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
            {
                Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), GameSessionName,
                    FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &USteamFpsGameInstance::OnRegisterLocalPlayerComplete));
            }
        }
        else
        {
            // We either failed or there is only a single local user
            FinishSessionCreation(bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
        }
    }
}

/** Initiates the session searching */
bool USteamFpsGameInstance::FindSessions(ULocalPlayer* PlayerOwner, bool bFindLAN)
{
    bool bResult = false;

    check(PlayerOwner != nullptr);
    if (PlayerOwner)
    {
        ASteamFpsGameSession* const GameSession = GetGameSession();
        if (GameSession)
        {
            GameSession->OnFindSessionsComplete().RemoveAll(this);
            OnSearchSessionsCompleteDelegateHandle = GameSession->OnFindSessionsComplete().AddUObject(this, &USteamFpsGameInstance::OnSearchSessionsComplete);

            GameSession->FindSessions(PlayerOwner->GetPreferredUniqueNetId(), GameSessionName, bFindLAN, true);

            bResult = true;
        }
    }

    return bResult;
}

/** Callback which is intended to be called upon finding sessions */
void USteamFpsGameInstance::OnSearchSessionsComplete(bool bWasSuccessful)
{
    ASteamFpsGameSession* const Session = GetGameSession();
    if (Session)
    {
        Session->OnFindSessionsComplete().Remove(OnSearchSessionsCompleteDelegateHandle);
    }
}

bool USteamFpsGameInstance::Tick(float DeltaSeconds)
{
    // Dedicated server doesn't need to worry about game state
    if (IsRunningDedicatedServer() == true)
    {
        return true;
    }

    MaybeChangeState();

    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

    if (CurrentState != ShooterGameInstanceState::WelcomeScreen)
    {
        // If at any point we aren't licensed (but we are after welcome screen) bounce them back to the welcome screen
        if (!bIsLicensed && CurrentState != ShooterGameInstanceState::None && !ShooterViewport->IsShowingDialog())
        {
            const FText ReturnReason = NSLOCTEXT("ProfileMessages", "NeedLicense", "The signed in users do not have a license for this game. Please purchase ShooterGame from the Xbox Marketplace or sign in a user with a valid license.");
            const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

            ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), ShooterGameInstanceState::WelcomeScreen);
        }

        // Show controller disconnected dialog if any local players have an invalid controller
        if (ShooterViewport != NULL &&
            !ShooterViewport->IsShowingDialog())
        {
            for (int i = 0; i < LocalPlayers.Num(); ++i)
            {
                if (LocalPlayers[i] && LocalPlayers[i]->GetControllerId() == -1)
                {
                    ShooterViewport->ShowDialog(
                        LocalPlayers[i],
                        EShooterDialogType::ControllerDisconnected,
                        FText::Format(NSLOCTEXT("ProfileMessages", "PlayerReconnectControllerFmt", "Player {0}, please reconnect your controller."), FText::AsNumber(i + 1)),
#ifdef PLATFORM_PS4
                        NSLOCTEXT("DialogButtons", "PS4_CrossButtonContinue", "Cross Button - Continue"),
#else
                        NSLOCTEXT("DialogButtons", "AButtonContinue", "A - Continue"),
#endif
                        FText::GetEmpty(),
                        FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnControllerReconnectConfirm),
                        FOnClicked()
                    );
                }
            }
        }
    }

    // If we have a pending invite, and we are at the welcome screen, and the session is properly shut down, accept it
    if (PendingInvite.UserId.IsValid() && PendingInvite.bPrivilegesCheckedAndAllowed && CurrentState == ShooterGameInstanceState::PendingInvite)
    {
        IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
        IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

        if (Sessions.IsValid())
        {
            EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);

            if (SessionState == EOnlineSessionState::NoSession)
            {
                ULocalPlayer * NewPlayerOwner = GetFirstGamePlayer();

                if (NewPlayerOwner != nullptr)
                {
                    NewPlayerOwner->SetControllerId(PendingInvite.ControllerId);
                    NewPlayerOwner->SetCachedUniqueNetId(PendingInvite.UserId);
                    SetIsOnline(true);
                    JoinSession(NewPlayerOwner, PendingInvite.InviteResult);
                }

                PendingInvite.UserId.Reset();
            }
        }
    }

    return true;
}

bool USteamFpsGameInstance::HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld)
{
    bool const bOpenSuccessful = Super::HandleOpenCommand(Cmd, Ar, InWorld);
    if (bOpenSuccessful)
    {
        GotoState(ShooterGameInstanceState::Playing);
    }

    return bOpenSuccessful;
}

void USteamFpsGameInstance::HandleSignInChangeMessaging()
{
    // Master user signed out, go to initial state (if we aren't there already)
    if (CurrentState != GetInitialState())
    {
#if SHOOTER_CONSOLE_UI
        // Display message on consoles
        const FText ReturnReason = NSLOCTEXT("ProfileMessages", "SignInChange", "Sign in status change occurred.");
        const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

        ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), GetInitialState());
#else								
        GotoInitialState();
#endif
    }
}

void USteamFpsGameInstance::HandleUserLoginChanged(int32 GameUserIndex, ELoginStatus::Type PreviousLoginStatus, ELoginStatus::Type LoginStatus, const FUniqueNetId& UserId)
{
    const bool bDowngraded = (LoginStatus == ELoginStatus::NotLoggedIn && !GetIsOnline()) || (LoginStatus != ELoginStatus::LoggedIn && GetIsOnline());

    UE_LOG(LogOnline, Log, TEXT("HandleUserLoginChanged: bDownGraded: %i"), (int)bDowngraded);

    TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
    bIsLicensed = GenericApplication->ApplicationLicenseValid();

    // Find the local player associated with this unique net id
    ULocalPlayer * LocalPlayer = FindLocalPlayerFromUniqueNetId(UserId);

    // If this user is signed out, but was previously signed in, punt to welcome (or remove splitscreen if that makes sense)
    if (LocalPlayer != NULL)
    {
        if (bDowngraded)
        {
            UE_LOG(LogOnline, Log, TEXT("HandleUserLoginChanged: Player logged out: %s"), *UserId.ToString());

            LabelPlayerAsQuitter(LocalPlayer);

            // Check to see if this was the master, or if this was a split-screen player on the client
            if (LocalPlayer == GetFirstGamePlayer() || GetIsOnline())
            {
                HandleSignInChangeMessaging();
            }
            else
            {
                // Remove local split-screen players from the list
                RemoveExistingLocalPlayer(LocalPlayer);
            }
        }
    }
}

void USteamFpsGameInstance::HandleAppWillDeactivate()
{
    if (CurrentState == ShooterGameInstanceState::Playing)
    {
        // Just have the first player controller pause the game.
        UWorld* const GameWorld = GetWorld();
        if (GameWorld)
        {
            // protect against a second pause menu loading on top of an existing one if someone presses the Jewel / PS buttons.
            bool bNeedsPause = true;
            for (FConstControllerIterator It = GameWorld->GetControllerIterator(); It; ++It)
            {
                auto Controller = Cast<ASteamFpsPlayerController>(*It);
                if (Controller && (Controller->IsPaused() || Controller->IsGameMenuVisible()))
                {
                    bNeedsPause = false;
                    break;
                }
            }

            if (bNeedsPause)
            {
                ASteamFpsPlayerController* const Controller = Cast<ASteamFpsPlayerController>(GameWorld->GetFirstPlayerController());
                if (Controller)
                {
                    Controller->ShowInGameMenu();
                }
            }
        }
    }
}

void USteamFpsGameInstance::HandleAppSuspend()
{
    // Players will lose connection on resume. However it is possible the game will exit before we get a resume, so we must kick off round end events here.
    UE_LOG(LogOnline, Warning, TEXT("USteamFpsGameInstance::HandleAppSuspend"));
    UWorld* const World = GetWorld();
    ASteamFpsGameState* const GameState = World != NULL ? World->GetGameState<ASteamFpsGameState>() : NULL;

    if (CurrentState != ShooterGameInstanceState::None && CurrentState != GetInitialState())
    {
        UE_LOG(LogOnline, Warning, TEXT("USteamFpsGameInstance::HandleAppSuspend: Sending round end event for players"));

        // Send round end events for local players
        for (int i = 0; i < LocalPlayers.Num(); ++i)
        {
            auto ShooterPC = Cast<ASteamFpsPlayerController>(LocalPlayers[i]->PlayerController);
            if (ShooterPC)
            {
                // Assuming you can't win if you quit early
                ShooterPC->ClientSendRoundEndEvent(false, GameState->ElapsedTime);
            }
        }
    }
}

void USteamFpsGameInstance::HandleAppResume()
{
    UE_LOG(LogOnline, Log, TEXT("USteamFpsGameInstance::HandleAppResume"));

    if (CurrentState != ShooterGameInstanceState::None && CurrentState != GetInitialState())
    {
        UE_LOG(LogOnline, Warning, TEXT("USteamFpsGameInstance::HandleAppResume: Attempting to sign out players"));

        for (int32 i = 0; i < LocalPlayers.Num(); ++i)
        {
            if (LocalPlayers[i]->GetCachedUniqueNetId().IsValid() && !IsLocalPlayerOnline(LocalPlayers[i]))
            {
                UE_LOG(LogOnline, Log, TEXT("USteamFpsGameInstance::HandleAppResume: Signed out during resume."));
                HandleSignInChangeMessaging();
                break;
            }
        }
    }
}

void USteamFpsGameInstance::HandleAppLicenseUpdate()
{
    TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
    bIsLicensed = GenericApplication->ApplicationLicenseValid();
}

void USteamFpsGameInstance::HandleSafeFrameChanged()
{
    UCanvas::UpdateAllCanvasSafeZoneData();
}

void USteamFpsGameInstance::RemoveExistingLocalPlayer(ULocalPlayer* ExistingPlayer)
{
    check(ExistingPlayer);
    if (ExistingPlayer->PlayerController != NULL)
    {
        // Kill the player
        ASteamFpsCharacter* MyPawn = Cast<ASteamFpsCharacter>(ExistingPlayer->PlayerController->GetPawn());
        if (MyPawn)
        {
            MyPawn->KilledBy(NULL);
        }
    }

    // Remove local split-screen players from the list
    RemoveLocalPlayer(ExistingPlayer);
}

void USteamFpsGameInstance::RemoveSplitScreenPlayers()
{
    // if we had been split screen, toss the extra players now
    // remove every player, back to front, except the first one
    while (LocalPlayers.Num() > 1)
    {
        ULocalPlayer* const PlayerToRemove = LocalPlayers.Last();
        RemoveExistingLocalPlayer(PlayerToRemove);
    }
}

FReply USteamFpsGameInstance::OnPairingUsePreviousProfile()
{
    // Do nothing (except hide the message) if they want to continue using previous profile
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

    if (ShooterViewport != nullptr)
    {
        ShooterViewport->HideDialog();
    }

    return FReply::Handled();
}

FReply USteamFpsGameInstance::OnPairingUseNewProfile()
{
    HandleSignInChangeMessaging();
    return FReply::Handled();
}

void USteamFpsGameInstance::HandleControllerPairingChanged(int GameUserIndex, const FUniqueNetId& PreviousUser, const FUniqueNetId& NewUser)
{
    UE_LOG(LogOnlineGame, Log, TEXT("USteamFpsGameInstance::HandleControllerPairingChanged GameUserIndex %d PreviousUser '%s' NewUser '%s'"),
        GameUserIndex, *PreviousUser.ToString(), *NewUser.ToString());

    if (CurrentState == ShooterGameInstanceState::WelcomeScreen)
    {
        // Don't care about pairing changes at welcome screen
        return;
    }

#if SHOOTER_CONSOLE_UI && PLATFORM_XBOXONE
    if (IgnorePairingChangeForControllerId != -1 && GameUserIndex == IgnorePairingChangeForControllerId)
    {
        // We were told to ignore
        IgnorePairingChangeForControllerId = -1;	// Reset now so there there is no chance this remains in a bad state
        return;
    }

    if (PreviousUser.IsValid() && !NewUser.IsValid())
    {
        // Treat this as a disconnect or signout, which is handled somewhere else
        return;
    }

    if (!PreviousUser.IsValid() && NewUser.IsValid())
    {
        // Treat this as a signin
        ULocalPlayer * ControlledLocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);

        if (ControlledLocalPlayer != NULL && !ControlledLocalPlayer->GetCachedUniqueNetId().IsValid())
        {
            // If a player that previously selected "continue without saving" signs into this controller, move them back to welcome screen
            HandleSignInChangeMessaging();
        }

        return;
    }

    // Find the local player currently being controlled by this controller
    ULocalPlayer * ControlledLocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);

    // See if the newly assigned profile is in our local player list
    ULocalPlayer * NewLocalPlayer = FindLocalPlayerFromUniqueNetId(NewUser);

    // If the local player being controlled is not the target of the pairing change, then give them a chance 
    // to continue controlling the old player with this controller
    if (ControlledLocalPlayer != nullptr && ControlledLocalPlayer != NewLocalPlayer)
    {
        UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

        if (ShooterViewport != nullptr)
        {
            ShooterViewport->ShowDialog(
                nullptr,
                EShooterDialogType::Generic,
                NSLOCTEXT("ProfileMessages", "PairingChanged", "Your controller has been paired to another profile, would you like to switch to this new profile now? Selecting YES will sign out of the previous profile."),
                NSLOCTEXT("DialogButtons", "YES", "A - YES"),
                NSLOCTEXT("DialogButtons", "NO", "B - NO"),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnPairingUseNewProfile),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnPairingUsePreviousProfile)
            );
        }
    }
#endif
}

void USteamFpsGameInstance::HandleControllerConnectionChange(bool bIsConnection, int32 Unused, int32 GameUserIndex)
{
    UE_LOG(LogOnlineGame, Log, TEXT("USteamFpsGameInstance::HandleControllerConnectionChange bIsConnection %d GameUserIndex %d"),
        bIsConnection, GameUserIndex);

    if (!bIsConnection)
    {
        // Controller was disconnected

        // Find the local player associated with this user index
        ULocalPlayer * LocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);

        if (LocalPlayer == NULL)
        {
            return;		// We don't care about players we aren't tracking
        }

        // Invalidate this local player's controller id.
        LocalPlayer->SetControllerId(-1);
    }
}

FReply USteamFpsGameInstance::OnControllerReconnectConfirm()
{
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
    if (ShooterViewport)
    {
        ShooterViewport->HideDialog();
    }

    return FReply::Handled();
}

TSharedPtr< const FUniqueNetId > USteamFpsGameInstance::GetUniqueNetIdFromControllerId(const int ControllerId)
{
    IOnlineIdentityPtr OnlineIdentityInt = Online::GetIdentityInterface();

    if (OnlineIdentityInt.IsValid())
    {
        TSharedPtr<const FUniqueNetId> UniqueId = OnlineIdentityInt->GetUniquePlayerId(ControllerId);

        if (UniqueId.IsValid())
        {
            return UniqueId;
        }
    }

    return nullptr;
}

void USteamFpsGameInstance::SetIsOnline(bool bInIsOnline)
{
    bIsOnline = bInIsOnline;
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

    if (OnlineSub)
    {
        for (int32 i = 0; i < LocalPlayers.Num(); ++i)
        {
            ULocalPlayer* LocalPlayer = LocalPlayers[i];

            TSharedPtr<const FUniqueNetId> PlayerId = LocalPlayer->GetPreferredUniqueNetId();
            if (PlayerId.IsValid())
            {
                OnlineSub->SetUsingMultiplayerFeatures(*PlayerId, bIsOnline);
            }
        }
    }
}

void USteamFpsGameInstance::TravelToSession(const FName& SessionName)
{
    // Added to handle failures when joining using quickmatch (handles issue of joining a game that just ended, i.e. during game ending timer)
    AddNetworkFailureHandlers();
    ShowLoadingScreen();
    GotoState(ShooterGameInstanceState::Playing);
    InternalTravelToSession(SessionName);
}

void USteamFpsGameInstance::SetIgnorePairingChangeForControllerId(const int32 ControllerId)
{
    IgnorePairingChangeForControllerId = ControllerId;
}

bool USteamFpsGameInstance::IsLocalPlayerOnline(ULocalPlayer* LocalPlayer)
{
    if (LocalPlayer == NULL)
    {
        return false;
    }
    const auto OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        const auto IdentityInterface = OnlineSub->GetIdentityInterface();
        if (IdentityInterface.IsValid())
        {
            auto UniqueId = LocalPlayer->GetCachedUniqueNetId();
            if (UniqueId.IsValid())
            {
                const auto LoginStatus = IdentityInterface->GetLoginStatus(*UniqueId);
                if (LoginStatus == ELoginStatus::LoggedIn)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool USteamFpsGameInstance::ValidatePlayerForOnlinePlay(ULocalPlayer* LocalPlayer)
{
    // Get the viewport
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

#if PLATFORM_XBOXONE
    if (CurrentConnectionStatus != EOnlineServerConnectionStatus::Connected)
    {
        // Don't let them play online if they aren't connected to Xbox LIVE
        if (ShooterViewport != NULL)
        {
            const FText Msg = NSLOCTEXT("NetworkFailures", "ServiceDisconnected", "You must be connected to the Xbox LIVE service to play online.");
            const FText OKButtonString = NSLOCTEXT("DialogButtons", "OKAY", "OK");

            ShooterViewport->ShowDialog(
                NULL,
                EShooterDialogType::Generic,
                Msg,
                OKButtonString,
                FText::GetEmpty(),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric)
            );
        }

        return false;
    }
#endif

    if (!IsLocalPlayerOnline(LocalPlayer))
    {
        // Don't let them play online if they aren't online
        if (ShooterViewport != NULL)
        {
            const FText Msg = NSLOCTEXT("NetworkFailures", "MustBeSignedIn", "You must be signed in to play online");
            const FText OKButtonString = NSLOCTEXT("DialogButtons", "OKAY", "OK");

            ShooterViewport->ShowDialog(
                NULL,
                EShooterDialogType::Generic,
                Msg,
                OKButtonString,
                FText::GetEmpty(),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric)
            );
        }

        return false;
    }

    return true;
}

FReply USteamFpsGameInstance::OnConfirmGeneric()
{
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
    if (ShooterViewport)
    {
        ShooterViewport->HideDialog();
    }

    return FReply::Handled();
}

void USteamFpsGameInstance::StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr< const FUniqueNetId > UserId)
{
    WaitMessageWidget = SNew(SShooterWaitDialog)
        .MessageText(NSLOCTEXT("NetworkStatus", "CheckingPrivilegesWithServer", "Checking privileges with server.  Please wait..."));

    if (GEngine && GEngine->GameViewport)
    {
        UGameViewportClient* const GVC = GEngine->GameViewport;
        GVC->AddViewportWidgetContent(WaitMessageWidget.ToSharedRef());
    }

    auto Identity = Online::GetIdentityInterface();
    if (Identity.IsValid() && UserId.IsValid())
    {
        Identity->GetUserPrivilege(*UserId, Privilege, Delegate);
    }
    else
    {
        // Can only get away with faking the UniqueNetId here because the delegates don't use it
        Delegate.ExecuteIfBound(FUniqueNetIdString(), Privilege, (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures);
    }
}

void USteamFpsGameInstance::CleanupOnlinePrivilegeTask()
{
    if (GEngine && GEngine->GameViewport && WaitMessageWidget.IsValid())
    {
        UGameViewportClient* const GVC = GEngine->GameViewport;
        GVC->RemoveViewportWidgetContent(WaitMessageWidget.ToSharedRef());
    }
}

void USteamFpsGameInstance::DisplayOnlinePrivilegeFailureDialogs(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
    // Show warning that the user cannot play due to age restrictions
    UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
    TWeakObjectPtr<ULocalPlayer> OwningPlayer;
    if (GEngine)
    {
        for (auto It = GEngine->GetLocalPlayerIterator(GetWorld()); It; ++It)
        {
            TSharedPtr<const FUniqueNetId> OtherId = (*It)->GetPreferredUniqueNetId();
            if (OtherId.IsValid())
            {
                if (UserId == (*OtherId))
                {
                    OwningPlayer = *It;
                }
            }
        }
    }

    if (ShooterViewport != NULL && OwningPlayer.IsValid())
    {
        if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure) != 0)
        {
            IOnlineExternalUIPtr ExternalUI = Online::GetExternalUIInterface();
            if (ExternalUI.IsValid())
            {
                ExternalUI->ShowAccountUpgradeUI(UserId);
            }
        }
        else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate) != 0)
        {
            ShooterViewport->ShowDialog(
                OwningPlayer.Get(),
                EShooterDialogType::Generic,
                NSLOCTEXT("OnlinePrivilegeResult", "RequiredSystemUpdate", "A required system update is available.  Please upgrade to access online features."),
                NSLOCTEXT("DialogButtons", "OKAY", "OK"),
                FText::GetEmpty(),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric)
            );
        }
        else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) != 0)
        {
            ShooterViewport->ShowDialog(
                OwningPlayer.Get(),
                EShooterDialogType::Generic,
                NSLOCTEXT("OnlinePrivilegeResult", "RequiredPatchAvailable", "A required game patch is available.  Please upgrade to access online features."),
                NSLOCTEXT("DialogButtons", "OKAY", "OK"),
                FText::GetEmpty(),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric)
            );
        }
        else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure) != 0)
        {
            ShooterViewport->ShowDialog(
                OwningPlayer.Get(),
                EShooterDialogType::Generic,
                NSLOCTEXT("OnlinePrivilegeResult", "AgeRestrictionFailure", "Cannot play due to age restrictions!"),
                NSLOCTEXT("DialogButtons", "OKAY", "OK"),
                FText::GetEmpty(),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric)
            );
        }
        else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) != 0)
        {
            ShooterViewport->ShowDialog(
                OwningPlayer.Get(),
                EShooterDialogType::Generic,
                NSLOCTEXT("OnlinePrivilegeResult", "UserNotFound", "Cannot play due invalid user!"),
                NSLOCTEXT("DialogButtons", "OKAY", "OK"),
                FText::GetEmpty(),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric)
            );
        }
        else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::GenericFailure) != 0)
        {
            ShooterViewport->ShowDialog(
                OwningPlayer.Get(),
                EShooterDialogType::Generic,
                NSLOCTEXT("OnlinePrivilegeResult", "GenericFailure", "Cannot play online.  Check your network connection."),
                NSLOCTEXT("DialogButtons", "OKAY", "OK"),
                FText::GetEmpty(),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric),
                FOnClicked::CreateUObject(this, &USteamFpsGameInstance::OnConfirmGeneric)
            );
        }
    }
}

void USteamFpsGameInstance::OnRegisterLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
    FinishSessionCreation(Result);
}

void USteamFpsGameInstance::FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        // Travel to the specified match URL
        GetWorld()->ServerTravel(TravelURL);
    }
    else
    {
        FText ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.");
        FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
        ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
    }
}

void USteamFpsGameInstance::BeginHostingQuickMatch()
{
    ShowLoadingScreen();
    GotoState(ShooterGameInstanceState::Playing);

    // Travel to the specified match URL
    GetWorld()->ServerTravel(TEXT("/Game/Maps/Highrise?game=TDM?listen"));
}

