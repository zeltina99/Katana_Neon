// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/BTTask_BossPhaseAction.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GAS/Tags/KNStatsTags.h"

#pragma region 기본 생성자 및 초기화 구현
UBTTask_BossPhaseAction::UBTTask_BossPhaseAction()
{
    NodeName = TEXT("보스 페이즈 전환 연출");
    bNotifyTick = false;
}
#pragma endregion 기본 생성자 및 초기화 구현

#pragma region 태스크 오버라이드 구현
EBTNodeResult::Type UBTTask_BossPhaseAction::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    const AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    ACharacter* BossChar = Cast<ACharacter>(Controller->GetPawn());
    if (!BossChar) return EBTNodeResult::Failed;

    UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(BossChar);
    if (!ASC) return EBTNodeResult::Failed;

    // 1. 전환 중 피격 방지를 위해 무적 태그 부여
    ASC->AddLooseGameplayTag(KatanaNeon::State::Combat::Invincible);

    // 2. 전환 몽타주 재생 (nullptr이면 연출 없이 즉시 진행)
    if (PhaseTransitionMontage)
    {
        BossChar->PlayAnimMontage(PhaseTransitionMontage);
    }

    // 3. InvincibleDuration 경과 후 무적 태그 제거
    // WeakLambda로 BossChar 파괴 시 크래시를 원천 차단합니다.
    FTimerHandle TimerHandle;
    BossChar->GetWorldTimerManager().SetTimer(
        TimerHandle,
        FTimerDelegate::CreateWeakLambda(BossChar, [ASC]()
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
#pragma endregion 태스크 오버라이드 구현
