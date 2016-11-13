// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

template <typename T>
static FORCEINLINE FString GetEnumAsString(const FString& typeName, T val)
{
    auto* ptr = FindObject<UEnum>(ANY_PACKAGE, *typeName, true);
    if (ptr != nullptr)
    {
        return ptr->GetEnumName(static_cast<int32>(val));
    }

    return FString("Unknown.");
}

// Helpers to get components, controllers, etc.
#define V_GET_RUNTIME_COMPONENT(actor, type) Cast<type>(actor->GetComponentByClass(type::StaticClass()))
#define V_GET_LOCAL_PLAYER(context, type)   Cast<type>(context->GetWorld()->GetFirstPlayerController())

// Helper for quickly iterating over all actors in the world of a given type (slow, but useful in game mode startups)
#define V_FOREACH_WORLD_ACTOR(worldContext, actorType) for (TActorIterator<actorType> iter(worldContext); iter; ++iter)

// Multiplayer helpers
#define V_GET_NETMODE_WORLD() (((GEngine == nullptr) || (GetWorld() == nullptr)) ? NM_MAX : GEngine->GetNetMode(GetWorld()))
#define V_NETMODE_IS(mode) (V_GET_NETMODE_WORLD() == mode)

// Pawn helpers
#define V_PAWN_ROLE_IS(pawn, role) ((pawn != nullptr) && (pawn->Role == role))
#define V_PAWN_ROLE_GTE(pawn, role) ((pawn != nullptr) && (pawn->Role >= role))

// Formatting macros to make logging output consistent.
#define V_FORMAT_VECTOR(value)      FString::Printf(TEXT("%.1f, %.1f, %.1f"), value.X, value.Y, value.Z)
#define V_FORMAT_ROTATOR(value)     FString::Printf(TEXT("Yaw=%.1f, Pitch=%.1f, Roll=%.1f"), value.Yaw, value.Pitch, value.Roll)
#define V_FORMAT_VECTOR2D(value)    FString::Printf(TEXT("%.1f, %.1f"), value.X, value.Y)
#define V_FORMAT_BOOL(value)        ((value) ? TEXT("TRUE") : TEXT("FALSE"))
#define V_FORMAT_ROLE(role)         ((role == ROLE_Authority) ? TEXT("Authority") : (role == ROLE_AutonomousProxy) ? TEXT("AutonomousProxy") : (role == ROLE_SimulatedProxy) ? TEXT("SimulatedProxy") : TEXT("None"))
#define V_FORMAT_ENUM(type, value)  GetEnumAsString<type>(TEXT( #type ), value)
#define V_FORMAT_ACTOR(actor)       FString::Printf(TEXT("%s"), (actor!=nullptr)?*actor->GetName():TEXT("NULL"))

// Macro to disallow copy operator.
#define V_CLASS_NOCOPY(type)  type & operator=(const type&) = delete
#define V_QUICK_PRINT(message, displayTime) GEngine->AddOnScreenDebugMessage(-1, displayTime, FColor::White, message)

