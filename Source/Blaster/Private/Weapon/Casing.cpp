// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Casing.h"
#include <Kismet/GameplayStatics.h>
#include "Sound/SoundCue.h"

// Sets default values
ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	//GetActorForwardVector�ǵ�λ���� ShellEjectionImpulse����������С ���ݲ�ͬ���������е���
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);//���һ���ӵ�X�᷽�������

}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	Destroy();

	//// ����һ���ӳ� 3 ���ִ�����ٵĶ�ʱ��
	//GetWorld()->GetTimerManager().SetTimer(
	//	DestroyTimerHandle,
	//	this,               
	//	&ACasing::DestroyCasing,  
	//	3.0f,               
	//	false               
	//);
}

// �����ӳ����ٵĺ���
void ACasing::DestroyCasing()
{
	Destroy();
}



