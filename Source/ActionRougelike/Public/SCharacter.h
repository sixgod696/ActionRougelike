// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SAttributeComponent.h"
#include "SInteractionComponent.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class USInteractionComponent;
class UAnimMontage;
class USAttributeComponent;

UCLASS()
class ACTIONROUGELIKE_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DashProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BlackHoleProjectileClass;
	
	UPROPERTY(EditAnywhere,Category="Attack")
	UAnimMontage* AttackAnim;

	FTimerHandle TimeHandle_PrimaryAttack;
	FTimerHandle TimeHandle_Dash;
	FTimerHandle TimeHandle_BlackHoleAttack;
	
public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArmComp;
	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere)
	USInteractionComponent* InteractionComp;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	USAttributeComponent* AttributeComp;

	void SpawnProjectile(TSubclassOf<AActor> ClassToSpawn);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);

	

	void PrimaryInteract();

	void PrimaryAttack();
	void PrimaryAttack_TimeElapsed();

	void Dash();
	void Dash_TimeElapsed();

	void BlackHoleAttack();
	void BlackHoleAttack_TimeElapsed();

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor,USAttributeComponent* OwningComp,float NewHealth,float Delta);

	virtual void PostInitializeComponents() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
