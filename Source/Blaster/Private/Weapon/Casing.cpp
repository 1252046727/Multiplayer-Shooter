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
	//GetActorForwardVector是单位向量 ShellEjectionImpulse是作用力大小 根据不同的武器进行调整
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);//添加一个子弹X轴方向的推力

}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	Destroy();

	//// 设置一个延迟 3 秒后执行销毁的定时器
	//GetWorld()->GetTimerManager().SetTimer(
	//	DestroyTimerHandle,
	//	this,               
	//	&ACasing::DestroyCasing,  
	//	3.0f,               
	//	false               
	//);
}

// 定义延迟销毁的函数
void ACasing::DestroyCasing()
{
	Destroy();
}



