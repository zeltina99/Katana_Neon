// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/BTTask_BossPhaseAction.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GAS/Tags/KNStatsTags.h"

UBTTask_BossPhaseAction::UBTTask_BossPhaseAction()
{
    NodeName = TEXT("보스 페이즈 전환 연출");
    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_BossPhaseAction::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    ACharacter* BossChar = Cast<ACharacter>(Controller->GetPawn());
    if (!BossChar) return EBTNodeResult::Failed;

    UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(BossChar);
    if (!ASC) return EBTNodeResult::Failed;

    // 1. 전환 중 무적 태그 부여
    ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::Invincible);

    // 2. 전환 몽타주 재생
    if (PhaseTransitionMontage)
    {
        BossChar->PlayAnimMontage(PhaseTransitionMontage);
    }

    // 3. InvincibleDuration 후 무적 해제 및 태스크 완료
    FTimerHandle TimerHandle;
    BossChar->GetWorldTimerManager().SetTimer(
        TimerHandle,
        FTimerDelegate::CreateWeakLambda(BossChar, [ASC, &OwnerComp, this]()
            {
                if (ASC && ASC->HasMatchingGameplayTag(KatanaNeon::State::Combat::Invincible))
                {
                    ASC->RemoveLooseGameplayTag(KatanaNeon::State::Combat::Invincible);
                }
            }),
        InvincibleDuration,
        false);

    return EBTNodeResult::Succeeded;
}
