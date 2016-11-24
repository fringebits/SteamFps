// Copyright (C) 2016 V1 Interactive Inc. - All Rights Reserved.
// $Id$ 
#pragma once

#include <vector>
#include <memory>
#include <functional>

// T: enum type
// K: context type
template <typename T, typename K>
class GenericState
{
public:
    GenericState(T state)
        : m_state(state)
        , m_timer(0.0f)
    {
    }

    virtual bool CanTransition(K* context, T next)
    {
        // This function used to block transition to next state while this state 'completes.
        return true;
    }

    virtual void OnEnter(K* context)
    {
        V_LOG_S(General, "==> OnEnter(0x%08x, %s)", reinterpret_cast<void*>(context), *V_FORMAT_ENUM(T, m_state));
        m_timer = 0.0f;
    }

    virtual void Tick(K* context, float dT)
    {
        m_timer += dT;
    }

    virtual void OnExit(K* context)
    {
        V_LOG_S(General, "<== OnExit(0x%08x, %s)", reinterpret_cast<void*>(context), *V_FORMAT_ENUM(T, m_state));
    }

    bool IsElapsed(float delta) const { return m_timer >= delta; }

private:
    T     m_state;
    float m_timer;
};

// T: enum type
// K: context type
template <typename T, typename K, typename S>
class GenericMachine
{
public:
    GenericMachine(size_t numStates)
        : m_states(numStates)
        , m_verbose(false)
    {
    }

    void Update(K* context, float dT)
    {
        auto cs = GetStatePointer(m_current);     // cs != null implies we've been initialized.
        V_CHECK(cs != nullptr);

        if (m_desired == m_current)
        {
            cs->Tick(context, dT);
            return;
        }

        // Handle state transition.
        auto next = m_funcGetNextState();

        // If we can't transition, just tick and exit.
        if (!cs->CanTransition(context, next))
        {
            cs->Tick(context, dT);
            return;
        }

        auto ns = m_states[(int)next];
        V_ASSERT(ns != nullptr, "Next state pointer must be non-null.");

        V_LOG_S(General, "StateTransition: %s -> (%s,%s)", 
            *GetStateString(m_current),
            *GetStateString(next),
            *GetStateString(m_desired));

        if (m_verbose)
        {
            UKismetSystemLibrary::PrintString(
                context,
                FString::Printf(TEXT("StateTransition: %s -> (%s, %s)"), *GetStateString(m_current), *GetStateString(next), *GetStateString(m_desired)),
                true, 
                false, 
                FColor::Yellow,
                5.0f);
        }

        cs->OnExit(context);
        ns->OnEnter(context);
        m_current = next; // Update current state value.
        ns->Tick(context, dT);
    }
       
    bool IsIdle() const 
    { 
        return m_current == m_desired; 
    }

    void SetDesiredState(T state)
    {
        V_LOG_S(General, "SetDesiredState: (%s,%s) -> %s", 
            *GetStateString(m_current),
            *GetStateString(m_desired),
            *GetStateString(state));

        m_desired = state;
    }

    T GetCurrentState() const { return m_current; }
    T GetDesiredState() const { return m_desired; }

    std::shared_ptr<S> GetStatePointer(T state)
    {
        V_ASSERT((int)state >= 0, "Invalid state value. Less than zero.");
        V_ASSERT((int)state < (int)m_states.size(), "Invalid state value, (<%d)", (int)m_states.size());
        return m_states[(int)state];
    }

    virtual FString GetStateString(T state) const = 0; // { return V_FORMAT_ENUM(T, state);  }

protected:
    void InitializeState(T state, std::shared_ptr<S> ptr)
    {
        V_ASSERT((int)state >= 0, "Invalid state value. Less than zero.");
        V_ASSERT((int)state < (int)m_states.size(), "Invalid state value, (<%d)", (int)m_states.size());

        m_states[(int)state] = ptr;
    }

protected:
    bool m_verbose;
    std::function<T()> m_funcGetNextState;
    std::vector<std::shared_ptr<S>> m_states;
    T m_current;
    T m_desired;
};

// T: enum type
// K: context type
template <typename T>
class SimpleMachine
{
public:
    SimpleMachine()
        : m_verbose(false)
    { }

    bool IsInState(const T cs) const { return cs == m_current; }

    bool IsStable() const { return IsInState(m_desired); }

    T GetCurrentState() const { return m_current; }

    T GetDesiredState() const { return m_desired; }

    void SetDesiredState(T state)
    {
        V_LOG_S(GeneralLog, "SetDesiredState: (%s,%s) -> %s",
            *GetStateString(m_current),
            *GetStateString(m_desired),
            *GetStateString(state));

        m_desired = state;
    }

protected:
    virtual T GetNextState(const T cs, const T ds) const = 0;

    virtual void TickState(const T cs, float dT) = 0;

    virtual void OnEnterState(const T cs) = 0;

    virtual void OnExitState(const T cs) = 0;

    virtual FString GetStateString(T state) const = 0;

    virtual bool CanTransition(const T ns) { return true; }

    void TickMachine(float dT)
    {
        if (IsStable())
        {
            TickState(m_current, dT);
            return;
        }

        // Handle state transition.
        auto next = GetNextState(m_current, m_desired);

        // If we can't transition, just tick and exit.
        if (!CanTransition(next))
        {
            TickState(m_current, dT);
            return;
        }

        //if (m_verbose)
        //{
        //    UKismetSystemLibrary::PrintString(
        //        context,
        //        FString::Printf(TEXT("StateTransition: %s -> (%s, %s)"), *GetStateString(m_current), *GetStateString(next), *GetStateString(m_desired)),
        //        true,
        //        false,
        //        FColor::Yellow,
        //        5.0f);
        //}

        OnExitState(m_current);
        OnEnterState(next);
        m_current = next; // Update current state value.
        TickState(m_current, dT);
    }

protected:
    bool m_verbose;
    T m_current;
    T m_desired;
};
