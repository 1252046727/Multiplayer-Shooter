// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/RocketMovementComponent.h"

URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	//默认返回EHandleBlockingHitResult::Deflect
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}
void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//不应该停止移动，所以这里什么也不做
	// Rockets should not stop; only explode when their CollisionBox detects a hit
}