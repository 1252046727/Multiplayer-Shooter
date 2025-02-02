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
	//TryGetPawnOwner��ȡ��������˶���ʵ���󶨵� Pawn ʵ����ͨ����һ����ɫ����
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

//����ÿ֡�������ɫ��صĶ�������
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
	Speed = Velocity.Size();//�����ٶ������ĳ��ȣ�Ҳ���ǽ�ɫ�����ٶȡ�S��������һ��������ֵ����ʾ��ɫ��ˮƽ���ϵ����ٶȡ�

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//�����������ļ��ٶȸ�ʵ����������ļ��ٶ������𣨰���Ϊ�̶�ֵ������Ϊ0��
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;//��������ǿ��Ը��Ƶ�
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();
	bHoldingTheFlag = BlasterCharacter->IsHoldingTheFlag();

	// Offset Yaw for Strafing  ����YawOffset��Lean��δ��벻��Ҫ�ֶ�����
	//��ɫֱ������X�᷽��ʱ��Ϊ0
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	//��ɫ�ƶ��ķ���ΪX�᷽��ʱ��Ϊ0
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	//YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//�����-180 ��180,�����ֵ�������� [-180,180]��ֵ ���������·��
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;//���ϱ�YawOffset���������AD���ذ�ɨ���ʱ�򶯻����ɸ�ƽ��

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	//�Ե�ǰ����бֵ Lean ��Ŀ��ֵ Target ���в�ֵ��ʹ��б�仯����ƽ����
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	//�� Interp ������ [-90��, 90��] ��Χ��
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		//���LeftHandSocket���������꣬LeftHandSocket����������������
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		//������ʱ������Ӧ����������ֽ��е��������ƶ�
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		//OutPosition �� OutRotation �����ֲ�������ֹ����ռ��е�λ�ú���ת��
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())//ֻ�ڿ��ƽ�ɫ�ϵ���ǹ�ĳ���ʹ����׼��Ŀ�귽���ǹ�ĳ��������ͬ
		{
			bLocallyControlled = true;
			//��ǹ�����ֹ�����Transform
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			//���ֹ���Transform����׼Ŀ�����ת�����ֹ����ķ����������Ҫ�����෴�ķ���
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			//����ת���в�ֵ
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}

		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		////����ֵ��MuzzleFlash X ������������ϵ�еķ�������
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
