// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/BTTask_BossAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AI/KNBossController.h"

UBTTask_BossAttack::UBTTask_BossAttack()
{
    NodeName = TEXT("보스 공격 실행");
    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_BossAttack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AKNBossController* Controller = Cast<AKNBossController>(OwnerComp.GetAIOwner());
    if (!Controller) return EBTNodeResult::Failed;

    APawn* Boss = Controller->GetPawn();
    if (!Boss) return EBTNodeResult::Failed;

    // 타겟이 없으면 공격하지 않음
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(AKNBossController::BBKey_TargetPlayer));
    if (!Target) return EBTNodeResult::Failed;

    // 공격 전 플레이어 방향으로 회전
    const FVector Direction = (Target->GetActorLocation() - Boss->GetActorLocation()).GetSafeNormal2D();
    Boss->SetActorRotation(Direction.Rotation());

    // ASC를 통해 공격 어빌리티 활성화
    UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Boss);
    if (!ASC || !AttackAbilityTag.IsValid()) return EBTNodeResult::Failed;

    const bool bSuccess =
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AttackAbilityTag));

    if (bSuccess)
    {
        BB->SetValueAsBool(AKNBossController::BBKey_IsAttacking, true);

        // 쿨타임 후 bIsAttacking 해제 및 태스크 완료
        FTimerHandle CooldownHandle;
        Boss->GetWorldTimerManager().SetTimer(
            CooldownHandle,
            FTimerDelegate::CreateWeakLambda(Boss, [BB, &OwnerComp]()
                {
                    if (BB)
                    {
                        BB->SetValueAsBool(AKNBossController::BBKey_IsAttacking, false);
                    }
                }),
            AttackCooldown,
            false);

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
