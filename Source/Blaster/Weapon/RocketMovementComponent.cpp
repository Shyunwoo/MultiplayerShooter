// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"



URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
    Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

    return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TileSlice, const FVector& MoveDelta)
{
    //Rockets should not stop, only explode when their collisionbox detects a hit
}
