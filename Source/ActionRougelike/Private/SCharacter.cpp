// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionRougelike/Public/SCharacter.h"

#include "SDashProjectile.h"
#include "Camera/CameraComponent.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment((RootComponent));

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);

	InteractionComp = CreateDefaultSubobject<USInteractionComponent>("InteractionComp");

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	bUseControllerRotationYaw = false;

	
	
}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AttributeComp->OnHealthChanged.AddDynamic(this,&ASCharacter::OnHealthChanged);
	
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASCharacter::MoveForward(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	
	AddMovementInput(ControlRot.Vector(),value);
}

void ASCharacter::MoveRight(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	//X forward (red)
	//Y right (green)
	//Z up (blue)
	
	FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	
	
	AddMovementInput(RightVector,value);
}



void ASCharacter::PrimaryInteract()
{
	if(InteractionComp)
	{
		InteractionComp->PrimaryInteract();
	}
	
	
}

void ASCharacter::PrimaryAttack()
{
	PlayAnimMontage(AttackAnim);//攻击动画

	GetWorldTimerManager().SetTimer(TimeHandle_PrimaryAttack,this,&ASCharacter::PrimaryAttack_TimeElapsed,0.2f);//定时器，很有用

	// GetWorldTimerManager().ClearTimer(TimeHandle_PrimaryAttack);
}

void ASCharacter::PrimaryAttack_TimeElapsed()
{
	// //使用ensure监测指针是否为空，一般不用check。一般在最终游戏完成时需要确保删除掉
	// if(ensureAlways(ProjectileClass))//ensure确保ProjectileClass指针不为空，优点在于报错后不会结束程序，显示此处缺少指针后按F5还会继续.默认只会触发一次，ensureAlways可以一直触发
	// {
	// 	FVector HandLoction = GetMesh()->GetSocketLocation("Muzzle_01");
	//
	// 	FTransform SpawnTM = FTransform(GetControlRotation(),HandLoction);
	//
	// 	FActorSpawnParameters SpawnParams;
	// 	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//
	// 	SpawnParams.Instigator = this;
	//
	// 	GetWorld()->SpawnActor<AActor>(ProjectileClass,SpawnTM,SpawnParams);
	// }
	SpawnProjectile(ProjectileClass);
}

void ASCharacter::Dash()
{
	PlayAnimMontage(AttackAnim);
	//GetWorldTimerManager().SetTimer(TimeHandle_Dash,this,&ASCharacter::Dash_TimeElapsed,AttackAnimDelay);
	GetWorldTimerManager().SetTimer(TimeHandle_Dash,this,&ASCharacter::Dash_TimeElapsed,0.2f);
}

void ASCharacter::Dash_TimeElapsed()
{
	SpawnProjectile(DashProjectileClass);
}

void ASCharacter::BlackHoleAttack()
{
	PlayAnimMontage(AttackAnim);
	//GetWorldTimerManager().SetTimer(TimeHandle_BlackHoleAttack,this,&ASCharacter::Dash_TimeElapsed,AttackAnimDelay);
	GetWorldTimerManager().SetTimer(TimeHandle_BlackHoleAttack,this,&ASCharacter::BlackHoleAttack_TimeElapsed,0.2f);
}

void ASCharacter::BlackHoleAttack_TimeElapsed()
{
	SpawnProjectile(BlackHoleProjectileClass);
}

void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth,
	float Delta)
{
	if(NewHealth <= 0.0f && Delta < 0.0f)
	{
		APlayerController* PC =  Cast<APlayerController>(GetController());
		DisableInput(PC);
	}
}



void ASCharacter::SpawnProjectile(TSubclassOf<AActor> ClassToSpawn)
{
	if(ensureAlways(ClassToSpawn))
	{
		//获得手的位置
		FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");
		//设置spawn的参数
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;
		
		FCollisionShape Shape;
		Shape.SetSphere(20.0f);

		//ignore player
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);

		FVector TraceStart = CameraComp->GetComponentLocation();

		//endpoint在远离注视点时（不太远，仍然在miss时稍微向十字准线调整）
		FVector TraceEnd = CameraComp->GetComponentLocation() + (GetControlRotation().Vector()* 5000);

		FHitResult Hit;
		//如果遇到阻挡命中，就返回true
		if(GetWorld()->SweepSingleByObjectType(Hit,TraceStart,TraceEnd,FQuat::Identity,ObjParams,Shape,Params))
		{
			//用world里的撞击点覆盖traceEnd
			TraceEnd = Hit.ImpactPoint;
		}

		//在world中找到新的从Hand pointing到撞击点impact point的方向或旋转
		FRotator ProjRotation = FRotationMatrix::MakeFromX(TraceEnd - HandLocation).Rotator();

		FTransform SpawnTM = FTransform(ProjRotation,HandLocation);
		GetWorld()->SpawnActor<AActor>(ClassToSpawn,SpawnTM,SpawnParams);
	}
}


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Moveforward",this,&ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&ASCharacter::MoveRight);
	
	PlayerInputComponent->BindAxis("Turn",this,&APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp",this,&APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ACharacter::Jump);//IE_Pressed是我们在按下时触发，IE_Released是在松开时除发
	// //由于我们没有重写Roll,因此需要调用父级类&ACharacter

	
	PlayerInputComponent->BindAction("PrimaryAttack",IE_Pressed,this,&ASCharacter::PrimaryAttack);
	PlayerInputComponent->BindAction("PrimaryInteract",IE_Pressed,this,&ASCharacter::PrimaryInteract);

	PlayerInputComponent->BindAction("DashAttack",IE_Pressed,this,&ASCharacter::Dash);
	PlayerInputComponent->BindAction("BlackHoleAttack",IE_Pressed,this,&ASCharacter::BlackHoleAttack);
	
}

