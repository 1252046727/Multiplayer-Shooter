// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlasterComponents/CombatComponent.h"
UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

//在服务端执行
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;
	
	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;
	
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;
	
	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;
	
	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed, float AimSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
	InitialAimSpeed = AimSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

//在服务端执行
void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffAimSpeed, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
		if (Character->GetCombat())
		{
			Character->GetCombat()->SetBaseWalkSpeed(BuffBaseSpeed);
			Character->GetCombat()->SetAimWalkSpeed(BuffAimSpeed);
			if (Character->GetCombat()->GetbAiming())
			{
				Character->GetCharacterMovement()->MaxWalkSpeed = BuffAimSpeed;
			}
			else
			{
				Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
			}
		}
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed, BuffAimSpeed);
}

void UBuffComponent::ResetSpeeds()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	if (Character->GetCombat())
	{
		Character->GetCombat()->SetBaseWalkSpeed(InitialBaseSpeed);
		Character->GetCombat()->SetAimWalkSpeed(InitialAimSpeed);
		if (Character->GetCombat()->GetbAiming())
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = InitialAimSpeed;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		}
	}
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed, InitialAimSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed, float AimSpeed)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
		if (Character->GetCombat())
		{
			Character->GetCombat()->SetBaseWalkSpeed(BaseSpeed);
			Character->GetCombat()->SetAimWalkSpeed(AimSpeed);
			if (Character->GetCombat()->GetbAiming())
			{
				Character->GetCharacterMovement()->MaxWalkSpeed = AimSpeed;
			}
			else
			{
				Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
			}
		}
	}
}

//在服务端执行
void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::ResetJump()
{
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

