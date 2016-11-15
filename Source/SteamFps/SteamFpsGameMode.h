// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

#include <GameFramework/GameMode.h>
#include <functional>
#include "SteamFpsGameMode.generated.h"

struct TimedCallback 
{
    TimedCallback(FName _name, bool _runonce, float _period, std::function<void()>& _func)
        : name(_name)
        , runonce(_runonce)
        , period(_period)
        , runAt(_period)
        , executed(false)
        , func(_func)
    { }

    FName name;
    float period;
    float runAt;
    bool  executed;
    bool  runonce;
    std::function<void()> func;
};

UCLASS()
class ASteamFpsGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASteamFpsGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float dT) override;
    virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;

protected:
    void ExecuteOnceAfter(FName name, float time, std::function<void()> func);
    void ExecuteOnPeriod(FName name, float time, std::function<void()> func);

protected:
    float m_modeTimer; // time spent in current mode (since begin play)
    std::list<std::shared_ptr<TimedCallback>> m_timers;  // list of timers to process
};