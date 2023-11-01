// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTService_CheckAttackRange.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void USBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//检查AIPawn和目标actor之间的距离
	
	UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
	if(ensure(BlackBoardComp))
	{
		AActor* TargetActor = Cast<AActor>(BlackBoardComp->GetValueAsObject("TargetActor"));
		if(TargetActor)
		{
			AAIController* MyController = OwnerComp.GetAIOwner();
			if(ensure(MyController))
			{
				APawn* AIPawn = MyController->GetPawn();
				if(ensure(AIPawn))
				{
					float DistanceTo = FVector::Distance(TargetActor->GetActorLocation(),AIPawn->GetActorLocation());

					bool bwithinRange = DistanceTo <2000.0f;
					
					bool bHasLOS = false;
					if(bwithinRange)
					{
						bHasLOS = MyController->LineOfSightTo(TargetActor);
					}
					
					
					BlackBoardComp->SetValueAsBool(AttackRangeKey.SelectedKeyName,bwithinRange && bHasLOS);
				}
				
			}
		}
	}
}
