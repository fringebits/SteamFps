// Copyright (C) 2016 V1 Interactive Inc. - All Rights Reserved.
#pragma once

#include <functional>
#include <list>

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

class CallbackComponent
{
protected:
    CallbackComponent()
        : m_modeTimer(0.0f)
    { }

    void TickTimers(float dT)
    {
        m_modeTimer += dT;

        for (auto&& f : m_timers)
        {
            if (m_modeTimer < f->runAt)
            {
                continue;
            }

            if (f->runonce && f->executed)
            {
                // technically, we could remove this guy from the list.
                continue;
            }

            if (f->runonce)
            {
                V_LOG_S(GeneralLog, "Execute: %s @ %.1f RUNONCE", *(f->name.ToString()), m_modeTimer);
            }
            f->executed = true;
            f->func();
            f->runAt = m_modeTimer + f->period;
        }
    }

    void ExecuteOnceAfter(FName name, float time, std::function<void()> func)
    {
        m_timers.emplace_back(std::make_shared<TimedCallback>(name, true, time, func));
    }

    void ExecuteOnPeriod(FName name, float time, std::function<void()> func)
    {
        m_timers.emplace_back(std::make_shared<TimedCallback>(name, false, time, func));
    }

private:
    float m_modeTimer; // time spent in current mode (since begin play)
    std::list<std::shared_ptr<TimedCallback>> m_timers;  // list of timers to process
};
