// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"
#include "BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	//TryGetPawnOwner获取到的是与此动画实例绑定的 Pawn 实例，通常是一个角色对象。
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

//用于每帧更新与角色相关的动画数据
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();//计算速度向量的长度，也就是角色的总速度。S方法返回一个浮动数值，表示角色在水平面上的总速度。

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//这里计算出来的加速度跟实际物理意义的加速度有区别（按下为固定值，不按为0）
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;//这个变量是可以复制的
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();
	bHoldingTheFlag = BlasterCharacter->IsHoldingTheFlag();

	// Offset Yaw for Strafing  计算YawOffset和Lean这段代码不需要手动复制
	//角色直视世界X轴方向时才为0
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	//角色移动的方向为X轴方向时才为0
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	//YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//如果是-180 到180,这个插值函数不会 [-180,180]插值 而是走最短路径
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;//跟上边YawOffset的区别就是AD来回按扫射的时候动画过渡更平滑

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	//对当前的倾斜值 Lean 和目标值 Target 进行插值，使倾斜变化更加平滑。
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	//将 Interp 限制在 [-90°, 90°] 范围内
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		//获得LeftHandSocket的世界坐标，LeftHandSocket在武器骨骼中设置
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		//在运行时武器不应该相对于右手进行调整或者移动
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		//OutPosition 和 OutRotation 是左手插槽在右手骨骼空间中的位置和旋转。
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())//只在控制角色上调整枪的朝向，使得瞄准的目标方向和枪的朝向大致相同
		{
			bLocallyControlled = true;
			//持枪人右手骨骼的Transform
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			//右手骨骼Transform到瞄准目标的旋转（右手骨骼的方向向里，所以要看向相反的方向）
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			//对旋转进行插值
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}

		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		////返回值是MuzzleFlash X 轴在世界坐标系中的方向向量
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Orange);
	
	}
	bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride = BlasterCharacter->IsLocallyControlled() && 
		BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade && 
		BlasterCharacter->bFinishedSwapping;
	if (bFABRIKOverride)
	{
		bUseFABRIK = !BlasterCharacter->IsLocallyReloading();
	}
	bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
	bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
}
