// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"

#include "SGameplayInterface.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"


// Sets default values for this component's properties
USInteractionComponent::USInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USInteractionComponent::PrimaryInteract()
{
	FCollisionObjectQueryParams ObjectQuaryParams;
	ObjectQuaryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	AActor* MyOwner = GetOwner();
	
	

	FVector EyeLocation;
	FRotator EyeRotation;

	//查看源码可知默认的眼睛高度是一个固定值，并不是我们实际角色的眼镜高度，可以后续修复
	MyOwner->GetActorEyesViewPoint(EyeLocation,EyeRotation);//获取角色注视点
	

	FVector End = EyeLocation + (EyeRotation.Vector()*1000);
	
	// FHitResult Hit;
	// bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit,EyeLocation,End,ObjectQuaryParams);//通过点碰撞跟踪,只是一个像素点，不符合实际游戏内情况

	TArray<FHitResult> Hits;

	float Radius = 30.f;
	
	FCollisionShape Shape;
	Shape.SetSphere(Radius);
	
	//sweep,欧拉角会有万向锁现象，故而使用四元数FQuat(相当于rotation的升级版本，包含更多信息)，矩阵运算也更快
	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits,EyeLocation,End ,FQuat::Identity,ObjectQuaryParams,Shape);

	//设置测试绘图颜色
	FColor LineColor = bBlockingHit ? FColor::Green:FColor::Red;
	
	for(FHitResult Hit:Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if(HitActor)//判断是否碰撞到目标对象
			{
			if(HitActor->Implements<USGameplayInterface>())//如果实现了接口
				{
				APawn* MyPawn = Cast<APawn>(MyOwner);
			
				ISGameplayInterface::Execute_Interact(HitActor,MyPawn);
				DrawDebugSphere(GetWorld(),Hit.ImpactPoint,Radius,32,LineColor,false,2.0f);
				break;//及时退出，防止一次互动开启多个宝箱
				}
			}
		//调试测试,画出球体大小
		
	}
	

	//调试测试,画出视角线

	DrawDebugLine(GetWorld(),EyeLocation,End ,LineColor,false,2.0f,0,2.0f);

	
}