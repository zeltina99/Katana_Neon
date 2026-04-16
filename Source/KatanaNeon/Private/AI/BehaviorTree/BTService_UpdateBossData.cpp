// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/BTService_UpdateBossData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AI/KNBossController.h"
#include "GAS/Tags/KNStatsTags.h"

UBTService_UpdateBossData::UBTService_UpdateBossData()
{
    NodeName = TEXT("보스 데이터 갱신");
    // 0.1초마다 갱신 (틱 비용 최소화)
    Interval = 0.1f;
    RandomDeviation = 0.05f;
}

void UBTService_UpdateBossData::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AKNBossController* Controller = Cast<AKNBossController>(OwnerComp.GetAIOwner());
    if (!Controller) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    APawn* Boss = Controller->GetPawn();
    if (!Boss) return;

    // 플레이어 거리 갱신
    AActor* Target = Cast<AActor>(BB->GetValueAsObject(AKNBossController::BBKey_TargetPlayer));
    if (Target)
    {
        const float Dist = FVector::Dist(Boss->GetActorLocation(), Target->GetActorLocation());
        BB->SetValueAsFloat(AKNBossController::BBKey_DistToPlayer, Dist);
    }

    // 그로기 태그 확인 → bIsStunned 갱신
    if (UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Boss))
    {
        const bool bStunned =
            ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Groggy);
        BB->SetValueAsBool(AKNBossController::BBKey_IsStunned, bStunned);
    }
}
